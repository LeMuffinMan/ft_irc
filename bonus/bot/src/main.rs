mod bot;
mod chat_gpt;
mod wall_e;
mod glados;

use bot::Bot;
use chat_gpt::chat_gpt;
use wall_e::wall_e;
use glados::glados;


#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let timeout_ms = 0;
    let glados_handle = tokio::spawn(glados(timeout_ms));
    let wall_e_handle = tokio::spawn(wall_e(timeout_ms));
    let chat_gpt_handle = tokio::spawn(chat_gpt(timeout_ms));

    if let Err(e) = glados_handle.await {
        eprintln!("GladOS error: {}", e);
    }
    if let Err(e) = wall_e_handle.await {
        eprintln!("Wall-E error: {}", e);
    }
    if let Err(e) = chat_gpt_handle.await {
        eprintln!("Chat-GPT error: {}", e);
    }

    Ok(())
}
