use actix_web::{web, App, HttpServer, Responder, post, get};
use actix_files::Files;
use serde::{Deserialize, Serialize};
use std::sync::Mutex;
use uuid::Uuid;
use chrono::{Utc, Duration};
use std::collections::HashMap;
use std::sync::RwLock;

#[macro_use]
extern crate diesel;
extern crate dotenv;

use diesel::prelude::*;
use diesel::pg::PgConnection;
use dotenv::dotenv;
use std::env;

mod schema;
mod models;
mod handlers;

use models::{User, Post, Comment, File, Notification, Group, GroupMember};

#[database("postgres")]
struct DbConn(diesel::PgConnection);

#[derive(Debug, Clone)]
struct AppState {
    db: DbConn,
    jwt_secret: String,
    sensitive_words: RwLock<Vec<String>>,
}

#[actix_rt::main]
async fn main() -> std::io::Result<()> {
    dotenv().ok();
    let db_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    let conn = PgConnection::establish(&db_url).expect("Failed to connect to the database");

    let app_state = web::Data::new(AppState {
        db: DbConn(conn),
        jwt_secret: env::var("JWT_SECRET").unwrap_or("your_jwt_secret".to_string()),
        sensitive_words: RwLock::new(vec!["badword1".to_string(), "badword2".to_string()]),
    });

    HttpServer::new(move || {
        App::new()
            .app_data(app_state.clone())
            .service(handlers::register)
            .service(handlers::login)
            .service(handlers::forgot_password)
            .service(handlers::reset_password)
            .service(handlers::check_in)
            .service(handlers::upgrade)
            .service(handlers::create_post)
            .service(handlers::get_posts)
            .service(handlers::create_comment)
            .service(handlers::upload_file)
            .service(handlers::get_files)
            .service(handlers::get_notifications)
            .service(handlers::get_groups)
            .service(Files::new("/static", "static"))
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}