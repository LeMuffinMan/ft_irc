mod bot;
mod chat_gpt;
mod glados;
mod wall_e;

use bot::Bot;
use chat_gpt::chat_gpt;
use glados::glados;
use wall_e::wall_e;

/*
*
* @Brief Bot
*
* This program simulate asychronously 3 bots on the irc server.
* Each bot is launched in using tokio spawn and collecting their future,
* allowing bot to not block each others on I/O operations while they interact with
* several players
*
* We only await chat_gpt_handle as it return means the end of the game,
* so we can abort() other bots
*
*/
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let timeout_ms = 0;
    let glados_handle = tokio::spawn(glados(timeout_ms));
    let wall_e_handle = tokio::spawn(wall_e(timeout_ms));
    let chat_gpt_handle = tokio::spawn(chat_gpt(timeout_ms));

    if let Err(e) = chat_gpt_handle.await {
        println!("Chat-GPT error: {}", e);
    }
    glados_handle.abort();
    wall_e_handle.abort();

    Ok(())
}
