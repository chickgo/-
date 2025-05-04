package main

import (
	"fmt"
	"log"
	"net/http"
	"time"

	"github.com/gorilla/mux"
	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/sqlite"
	"github.com/dgrijalva/jwt-go"
)

var db *gorm.DB
var jwtKey = []byte("your_jwt_secret")

type App struct {
	Router *mux.Router
	DB     *gorm.DB
}

type User struct {
	gorm.Model
	Username     string `json:"username"`
	Email        string `json:"email"`
	Password     string `json:"password"`
	IsOnline     bool   `json:"is_online"`
	LastCheckin  time.Time `json:"last_checkin"`
	Points       int    `json:"points"`
	Level        int    `json:"level"`
	ResetToken   string `json:"reset_token"`
	ResetTokenExpiry time.Time `json:"reset_token_expiry"`
	Posts        []Post `gorm:"foreignkey:User_ID"`
	Comments     []Comment `gorm:"foreignkey:User_ID"`
	Files        []File `gorm:"foreignkey:User_ID"`
	Notifications []Notification `gorm:"foreignkey:User_ID"`
	Groups       []GroupMember `gorm:"foreignkey:User_ID"`
}

type Post struct {
	gorm.Model
	Content   string `json:"content"`
	DatePosted time.Time `json:"date_posted"`
	User_ID   uint `json:"user_id"`
	Comments  []Comment `gorm:"foreignkey:Post_ID"`
}

type Comment struct {
	gorm.Model
	Content   string `json:"content"`
	DatePosted time.Time `json:"date_posted"`
	User_ID   uint `json:"user_id"`
	Post_ID   uint `json:"post_id"`
}

type File struct {
	gorm.Model
	Filename   string `json:"filename"`
	Path       string `json:"path"`
	UploadDate time.Time `json:"upload_date"`
	User_ID    uint `json:"user_id"`
}

type Notification struct {
	gorm.Model
	Content   string `json:"content"`
	DateSent  time.Time `json:"date_sent"`
	IsRead    bool `json:"is_read"`
	User_ID   uint `json:"user_id"`
}

type Group struct {
	gorm.Model
	Name        string `json:"name"`
	Description string `json:"description"`
	CreatedAt   time.Time `json:"created_at"`
	Members     []GroupMember `gorm:"foreignkey:Group_ID"`
}

type GroupMember struct {
	gorm.Model
	User_ID  uint `json:"user_id"`
	Group_ID uint `json:"group_id"`
	JoinedAt time.Time `json:"joined_at"`
}

type Claims struct {
	Username string `json:"username"`
	jwt.StandardClaims
}

func (a *App) Initialize() {
	var err error
	a.DB, err = gorm.Open("sqlite3", "test.db")
	if err != nil {
		log.Fatal("Failed to connect database:", err)
	}
	a.DB.AutoMigrate(&User{}, &Post{}, &Comment{}, &File{}, &Notification{}, &Group{}, &GroupMember{})

	a.Router = mux.NewRouter()
	a.setRoutes()
}

func (a *App) setRoutes() {
	a.Router.HandleFunc("/register", a.register).Methods("POST")
	a.Router.HandleFunc("/login", a.login).Methods("POST")
	a.Router.HandleFunc("/forgot-password", a.forgotPassword).Methods("POST")
	a.Router.HandleFunc("/reset-password/{token}", a.resetPassword).Methods("POST")
	a.Router.HandleFunc("/check-in", a.checkIn).Methods("POST")
	a.Router.HandleFunc("/upgrade", a.upgrade).Methods("POST")
	a.Router.HandleFunc("/posts", a.getPosts).Methods("GET")
	a.Router.HandleFunc("/posts", a.createPost).Methods("POST")
	a.Router.HandleFunc("/comments/{post_id}", a.createComment).Methods("POST")
	a.Router.HandleFunc("/files", a.uploadFile).Methods("POST")
	a.Router.HandleFunc("/notifications", a.getNotifications).Methods("GET")
	a.Router.HandleFunc("/groups", a.getGroups).Methods("GET")
}

func (a *App) register(w http.ResponseWriter, r *http.Request) {
	var user User
	if err := a.DB.Where("username = ? OR email = ?", user.Username, user.Email).First(&user).Error; err == nil {
		respondWithError(w, http.StatusBadRequest, "User already exists")
		return
	}

	if err := json.NewDecoder(r.Body).Decode(&user); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(user.Password), bcrypt.DefaultCost)
	if err != nil {
		respondWithError(w, http.StatusInternalServerError, "Error hashing password")
		return
	}
	user.Password = string(hashedPassword)
	user.Points = 0
	user.Level = 1
	user.IsOnline = false

	a.DB.Create(&user)
	respondWithJSON(w, http.StatusCreated, user)
}

func (a *App) login(w http.ResponseWriter, r *http.Request) {
	var user User
	var creds struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}

	if err := json.NewDecoder(r.Body).Decode(&creds); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	if err := a.DB.Where("username = ?", creds.Username).First(&user).Error; err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid credentials")
		return
	}

	if err := bcrypt.CompareHashAndPassword([]byte(user.Password), []byte(creds.Password)); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid credentials")
		return
	}

	user.IsOnline = true
	a.DB.Save(&user)

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"username": user.Username,
		"exp":      time.Now().Add(time.Hour * 72).Unix(),
	})

	tokenString, err := token.SignedString(jwtKey)
	if err != nil {
		respondWithError(w, http.StatusInternalServerError, "Error generating token")
		return
	}

	respondWithJSON(w, http.StatusOK, map[string]string{"token": tokenString})
}

func (a *App) forgotPassword(w http.ResponseWriter, r *http.Request) {
	var user User
	var req struct {
		Email string `json:"email"`
	}

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	if err := a.DB.Where("email = ?", req.Email).First(&user).Error; err != nil {
		respondWithError(w, http.StatusBadRequest, "Email not found")
		return
	}

	token := generateResetToken()
	expires := time.Now().Add(time.Hour * 1)
	user.ResetToken = token
	user.ResetTokenExpiry = expires
	a.DB.Save(&user)

	// Send email with token (simplified)
	fmt.Printf("Password reset token sent to %s: %s\n", req.Email, token)
	respondWithJSON(w, http.StatusOK, map[string]string{"message": "Password reset email sent"})
}

func (a *App) resetPassword(w http.ResponseWriter, r *http.Request) {
	var user User
	var req struct {
		Password string `json:"password"`
	}

	token := mux.Vars(r)["token"]
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	if err := a.DB.Where("reset_token = ? AND reset_token_expiry > ?", token, time.Now()).First(&user).Error; err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid or expired token")
		return
	}

	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(req.Password), bcrypt.DefaultCost)
	if err != nil {
		respondWithError(w, http.StatusInternalServerError, "Error hashing password")
		return
	}

	user.Password = string(hashedPassword)
	user.ResetToken = ""
	user.ResetTokenExpiry = time.Time{}
	a.DB.Save(&user)

	respondWithJSON(w, http.StatusOK, map[string]string{"message": "Password reset successfully"})
}

func (a *App) checkIn(w http.ResponseWriter, r *http.Request) {
	var user User
	claims := parseJWT(r)

	if err := a.DB.First(&user, "username = ?", claims.Username).Error; err != nil {
		respondWithError(w, http.StatusUnauthorized, "Unauthorized")
		return
	}

	if user.LastCheckin.IsZero() || user.LastCheckin.Before(time.Now().Add(-24 * time.Hour)) {
		user.Points += 10
		user.LastCheckin = time.Now()
		a.DB.Save(&user)
		respondWithJSON(w, http.StatusOK, map[string]int{"points": user.Points})
	} else {
		respondWithError(w, http.StatusBadRequest, "Already checked in today")
	}
}

func (a *App) upgrade(w http.ResponseWriter, r *http.Request) {
	var user User
	claims := parseJWT(r)
	var req struct {
		Points int `json:"points"`
	}

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	if err := a.DB.First(&user, "username = ?", claims.Username).Error; err != nil {
		respondWithError(w, http.StatusUnauthorized, "Unauthorized")
		return
	}

	if user.Points >= req.Points {
		user.Points -= req.Points
		user.Level += 1
		a.DB.Save(&user)
		respondWithJSON(w, http.StatusOK, map[string]int{"level": user.Level, "points": user.Points})
	} else {
		respondWithError(w, http.StatusBadRequest, "Not enough points")
	}
}

func (a *App) getPosts(w http.ResponseWriter, r *http.Request) {
	var posts []Post
	a.DB.Preload("Comments").Find(&posts)
	respondWithJSON(w, http.StatusOK, posts)
}

func (a *App) createPost(w http.ResponseWriter, r *http.Request) {
	claims := parseJWT(r)
	var post Post

	if err := json.NewDecoder(r.Body).Decode(&post); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	post.User_ID = claims.UserID
	post.DatePosted = time.Now()
	a.DB.Create(&post)
	respondWithJSON(w, http.StatusCreated, post)
}

func (a *App) createComment(w http.ResponseWriter, r *http.Request) {
	claims := parseJWT(r)
	vars := mux.Vars(r)
	postID, err := strconv.Atoi(vars["post_id"])
	if err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid post ID")
		return
	}

	var comment Comment
	if err := json.NewDecoder(r.Body).Decode(&comment); err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid request payload")
		return
	}

	comment.User_ID = claims.UserID
	comment.Post_ID = uint(postID)
	comment.DatePosted = time.Now()
	a.DB.Create(&comment)
	respondWithJSON(w, http.StatusCreated, comment)
}

func (a *App) uploadFile(w http.ResponseWriter, r *http.Request) {
	claims := parseJWT(r)

	file, header, err := r.FormFile("file")
	if err != nil {
		respondWithError(w, http.StatusBadRequest, "Invalid file")
		return
	}
	defer file.Close()

	filePath := fmt.Sprintf("uploads/%s", header.Filename)
	out, err := os.Create(filePath)
	if err != nil {
		respondWithError(w, http.StatusInternalServerError, "Error saving file")
		return
	}
	defer out.Close()

	_, err = io.Copy(out, file)
	if err != nil {
		respondWithError(w, http.StatusInternalServerError, "Error saving file")
		return
	}

	newFile := File{
		Filename:   header.Filename,
		Path:       filePath,
		UploadDate: time.Now(),
		User_ID:    claims.UserID,
	}
	a.DB.Create(&newFile)
	respondWithJSON(w, http.StatusCreated, newFile)
}

func (a *App) getNotifications(w http.ResponseWriter, r *http.Request) {
	claims := parseJWT(r)
	var notifications []Notification

	a.DB.Where("user_id = ?", claims.UserID).Find(&notifications)
	respondWithJSON(w, http.StatusOK, notifications)
}

func (a *App) getGroups(w http.ResponseWriter, r *http.Request) {
	var groups []Group
	a.DB.Preload("Members").Find(&groups)
	respondWithJSON(w, http.StatusOK, groups)
}

func parseJWT(r *http.Request) jwt.MapClaims {
	tokenString := r.Header.Get("Authorization")
	token, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("Unexpected signing method: %v", token.Header["alg"])
		}
		return jwtKey, nil
	})

	if err != nil {
		respondWithError(r.Writer, http.StatusUnauthorized, "Invalid token")
		return nil
	}

	claims := token.Claims.(jwt.MapClaims)
	return claims
}

func respondWithError(w http.ResponseWriter, code int, message string) {
	respondWithJSON(w, code, map[string]string{"error": message})
}

func respondWithJSON(w http.ResponseWriter, code int, payload interface{}) {
	response, _ := json.Marshal(payload)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(code)
	w.Write(response)
}

func generateResetToken() string {
	b := make([]byte, 36)
	if _, err := rand.Read(b); err != nil {
		return ""
	}
	return fmt.Sprintf("%x", b)
}

func main() {
	a := App{}
	a.Initialize()

	defer a.DB.Close()

	log.Println("Server starting on :8080")
	log.Fatal(http.ListenAndServe(":8080", a.Router))
}