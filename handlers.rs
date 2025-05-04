use actix_web::{web, HttpResponse, Responder, post, get};
use serde::{Deserialize, Serialize};
use uuid::Uuid;
use chrono::{Utc, Duration};
use jsonwebtoken::{encode, decode, Header, Validation, DecodingKey, EncodingKey};
use crate::AppState;
use diesel::prelude::*;

#[derive(Serialize, Deserialize)]
struct UserRegistration {
    username: String,
    email: String,
    password: String,
}

#[derive(Serialize, Deserialize)]
struct UserLogin {
    username: String,
    password: String,
}

#[derive(Serialize, Deserialize)]
struct ForgotPassword {
    email: String,
}

#[derive(Serialize, Deserialize)]
struct ResetPassword {
    password: String,
}

#[derive(Serialize, Deserialize)]
struct CheckInRequest {
    username: String,
}

#[derive(Serialize, Deserialize)]
struct UpgradeRequest {
    points: i32,
}

#[derive(Serialize, Deserialize)]
struct CreatePostRequest {
    content: String,
}

#[derive(Serialize, Deserialize)]
struct CreateCommentRequest {
    content: String,
    post_id: i32,
}

#[derive(Serialize, Deserialize)]
struct UploadFileRequest {
    filename: String,
    path: String,
}

#[post("/register")]
async fn register(data: web::Data<AppState>, req: web::Json<UserRegistration>) -> impl Responder {
    use crate::schema::users;

    let hashed_password = bcrypt::hash(&req.password, bcrypt::DEFAULT_COST).unwrap();
    let new_user = models::NewUser {
        username: &req.username,
        email: &req.email,
        password: &hashed_password,
        points: 0,
        level: 1,
        is_online: false,
        last_checkin: None,
        reset_token: None,
        reset_token_expiry: None,
    };

    let conn = &data.db.0;
    diesel::insert_into(users::table)
        .values(&new_user)
        .execute(conn)
        .expect("Error saving new user");

    HttpResponse::Created().json(web::Json(json!({
        "message": "User registered successfully",
        "user": {
            "username": req.username,
            "email": req.email,
            "points": 0,
            "level": 1
        }
    })))
}

#[post("/login")]
async fn login(data: web::Data<AppState>, req: web::Json<UserLogin>) -> impl Responder {
    use crate::schema::users::dsl::*;

    let conn = &data.db.0;
    let user = users.filter(username.eq(&req.username)).first::<models::User>(conn).ok();

    match user {
        Some(user) => {
            if bcrypt::verify(&req.password, &user.password).unwrap_or(false) {
                let token = encode(
                    &Header::default(),
                    &Claims::new(user.id),
                    &EncodingKey::from_secret(data.jwt_secret.as_bytes()),
                ).unwrap();

                diesel::update(users.find(user.id))
                    .set(is_online.eq(true))
                    .execute(conn)
                    .expect("Error updating user online status");

                HttpResponse::Ok().json(web::Json(json!({
                    "message": "Login successful",
                    "token": token,
                    "user": {
                        "id": user.id,
                        "username": user.username,
                        "email": user.email,
                        "points": user.points,
                        "level": user.level
                    }
                })))
            } else {
                HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid credentials"})))
            }
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid credentials"}))),
    }
}

#[post("/forgot-password")]
async fn forgot_password(data: web::Data<AppState>, req: web::Json<ForgotPassword>) -> impl Responder {
    use crate::schema::users::dsl::*;

    let conn = &data.db.0;
    let user = users.filter(email.eq(&req.email)).first::<models::User>(conn).ok();

    match user {
        Some(mut user) => {
            let reset_token = Uuid::new_v4().to_string();
            let reset_token_expiry = Utc::now() + Duration::hours(1);

            diesel::update(users.find(user.id))
                .set((
                    models::reset_token.eq(&reset_token),
                    models::reset_token_expiry.eq(reset_token_expiry),
                ))
                .execute(conn)
                .expect("Error updating reset token");

            // Send email with reset token (simplified)
            println!("Password reset token sent to {}: {}", req.email, reset_token);

            HttpResponse::Ok().json(web::Json(json!({"message": "Password reset email sent"})))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Email not found"}))),
    }
}

#[post("/reset-password/{token}")]
async fn reset_password(data: web::Data<AppState>, req: web::Json<ResetPassword>, token: web::Path<String>) -> impl Responder {
    use crate::schema::users::dsl::*;

    let conn = &data.db.0;
    let user = users.filter(reset_token.eq(&token))
        .filter(reset_token_expiry.gt(Utc::now().naive_utc()))
        .first::<models::User>(conn)
        .ok();

    match user {
        Some(mut user) => {
            let hashed_password = bcrypt::hash(&req.password, bcrypt::DEFAULT_COST).unwrap();

            diesel::update(users.find(user.id))
                .set((
                    password.eq(&hashed_password),
                    reset_token.eq::<Option<String>>(None),
                    reset_token_expiry.eq::<Option<chrono::NaiveDateTime>>(None),
                ))
                .execute(conn)
                .expect("Error resetting password");

            HttpResponse::Ok().json(web::Json(json!({"message": "Password reset successfully"})))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid or expired token"}))),
    }
}

#[derive(Serialize)]
struct Claims {
    sub: String,
    exp: i64,
}

impl Claims {
    fn new(user_id: i32) -> Self {
        Self {
            sub: user_id.to_string(),
            exp: (Utc::now() + Duration::days(7)).timestamp(),
        }
    }
}

fn validate_token(token: &str, secret: &[u8]) -> Option<i32> {
    match decode::<Claims>(token, &DecodingKey::from_secret(secret), &Validation::default()) {
        Ok(token_data) => Some(token_data.claims.sub.parse().ok()?),
        Err(_) => None,
    }
}

#[post("/check-in")]
async fn check_in(data: web::Data<AppState>, req: web::Json<CheckInRequest>) -> impl Responder {
    use crate::schema::users::dsl::*;

    let conn = &data.db.0;
    let user = users.filter(username.eq(&req.username)).first::<models::User>(conn).ok();

    match user {
        Some(mut user) => {
            let last_checkin = user.last_checkin.unwrap_or(Utc::now().naive_utc() - Duration::days(1));
            if last_checkin < Utc::now().naive_utc() - Duration::days(1) {
                diesel::update(users.find(user.id))
                    .set((
                        points.eq(user.points + 10),
                        last_checkin.eq(Utc::now().naive_utc()),
                    ))
                    .execute(conn)
                    .expect("Error updating user points");

                HttpResponse::Ok().json(web::Json(json!({
                    "message": "Checked in successfully",
                    "points": user.points + 10
                })))
            } else {
                HttpResponse::BadRequest().json(web::Json(json!({"message": "Already checked in today"})))
            }
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "User not found"}))),
    }
}

#[post("/upgrade")]
async fn upgrade(data: web::Data<AppState>, req: web::Json<UpgradeRequest>, token: web::Header<String>) -> impl Responder {
    let user_id = validate_token(&token, data.jwt_secret.as_bytes());

    match user_id {
        Some(id) => {
            use crate::schema::users::dsl::*;

            let conn = &data.db.0;
            let user = users.find(id).first::<models::User>(conn).ok();

            match user {
                Some(mut user) => {
                    if user.points >= req.points {
                        diesel::update(users.find(user.id))
                            .set((
                                points.eq(user.points - req.points),
                                models::level.eq(user.level + 1),
                            ))
                            .execute(conn)
                            .expect("Error updating user level");

                        HttpResponse::Ok().json(web::Json(json!({
                            "message": "Upgraded successfully",
                            "level": user.level + 1,
                            "points": user.points - req.points
                        })))
                    } else {
                        HttpResponse::BadRequest().json(web::Json(json!({"message": "Not enough points"})))
                    }
                }
                None => HttpResponse::BadRequest().json(web::Json(json!({"message": "User not found"}))),
            }
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid token"}))),
    }
}

#[post("/posts")]
async fn create_post(data: web::Data<AppState>, req: web::Json<CreatePostRequest>, token: web::Header<String>) -> impl Responder {
    let user_id = validate_token(&token, data.jwt_secret.as_bytes());

    match user_id {
        Some(id) => {
            use crate::schema::posts;

            let new_post = models::NewPost {
                content: &req.content,
                user_id: id,
            };

            let conn = &data.db.0;
            let post = diesel::insert_into(posts::table)
                .values(&new_post)
                .get_result::<models::Post>(conn)
                .expect("Error creating post");

            HttpResponse::Created().json(web::Json(json!({
                "message": "Post created",
                "post": post
            })))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid token"}))),
    }
}

#[get("/posts")]
async fn get_posts(data: web::Data<AppState>) -> impl Responder {
    use crate::schema::posts::dsl::*;

    let conn = &data.db.0;
    let posts = posts.order(created_at.desc()).load::<models::Post>(conn).expect("Error loading posts");

    HttpResponse::Ok().json(web::Json(posts))
}

#[post("/comments/{post_id}")]
async fn create_comment(data: web::Data<AppState>, req: web::Json<CreateCommentRequest>, token: web::Header<String>, post_id: web::Path<i32>) -> impl Responder {
    let user_id = validate_token(&token, data.jwt_secret.as_bytes());

    match user_id {
        Some(id) => {
            use crate::schema::comments;

            let new_comment = models::NewComment {
                content: &req.content,
                user_id: id,
                post_id: post_id.into_inner(),
            };

            let conn = &data.db.0;
            let comment = diesel::insert_into(comments::table)
                .values(&new_comment)
                .get_result::<models::Comment>(conn)
                .expect("Error creating comment");

            HttpResponse::Created().json(web::Json(json!({
                "message": "Comment created",
                "comment": comment
            })))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid token"}))),
    }
}

#[post("/files")]
async fn upload_file(data: web::Data<AppState>, req: web::Json<UploadFileRequest>, token: web::Header<String>) -> impl Responder {
    let user_id = validate_token(&token, data.jwt_secret.as_bytes());

    match user_id {
        Some(id) => {
            use crate::schema::files;

            let new_file = models::NewFile {
                filename: &req.filename,
                path: &req.path,
                user_id: id,
            };

            let conn = &data.db.0;
            let file = diesel::insert_into(files::table)
                .values(&new_file)
                .get_result::<models::File>(conn)
                .expect("Error uploading file");

            HttpResponse::Created().json(web::Json(json!({
                "message": "File uploaded",
                "file": file
            })))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid token"}))),
    }
}

#[get("/files")]
async fn get_files(data: web::Data<AppState>, token: web::Header<String>) -> impl Responder {
    let user_id = validate_token(&token, data.jwt_secret.as_bytes());

    match user_id {
        Some(id) => {
            use crate::schema::files::dsl::*;

            let conn = &data.db.0;
            let files = files.filter(user_id.eq(id)).load::<models::File>(conn).expect("Error loading files");

            HttpResponse::Ok().json(web::Json(files))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid token"}))),
    }
}

#[get("/notifications")]
async fn get_notifications(data: web::Data<AppState>, token: web::Header<String>) -> impl Responder {
    let user_id = validate_token(&token, data.jwt_secret.as_bytes());

    match user_id {
        Some(id) => {
            use crate::schema::notifications::dsl::*;

            let conn = &data.db.0;
            let notifications = notifications.filter(user_id.eq(id)).load::<models::Notification>(conn).expect("Error loading notifications");

            HttpResponse::Ok().json(web::Json(notifications))
        }
        None => HttpResponse::BadRequest().json(web::Json(json!({"message": "Invalid token"}))),
    }
}

#[get("/groups")]
async fn get_groups(data: web::Data<AppState>) -> impl Responder {
    use crate::schema::groups::dsl::*;

    let conn = &data.db.0;
    let groups = groups.load::<models::Group>(conn).expect("Error loading groups");

    HttpResponse::Ok().json(web::Json(groups))
}