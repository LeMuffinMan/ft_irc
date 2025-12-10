mod bot;

use bot::Bot;

async fn glados(timeout_ms: u64) -> Result<(), Box<dyn std::error::Error>> {
    let mut bot = Bot::connect(6667).await?;
    let nick = "GladOS";
    let riddle = "PRIVMSG #ApertureScience :Alright, listen carefully, because I’m only going to say this once.

You stand before two doors.
One leads to cake.
The other leads to a room full of neurotoxin gaz, and absolutely no cake.

Choose wisely, or don’t.
The outcome is equally informative.
For science.

You monster.

[1] -> The Cake door
[2] -> The neurotoxin gaz and absolutely no cake door.
\r\n";
    let answer = "2";
    // std::thread::sleep(std::time::Duration::from_secs(1));
    bot.authenticate(nick.to_string(), timeout_ms).await?;


    bot.try_expect("JOIN #ApertureScience\r\n",
        "GladOS JOIN #ApertureScience",
        "GladOS failed to join channel",
        timeout_ms).await?;
    if let Some(nick_player) = bot.get_user_nick(timeout_ms).await {
        println!("nick_player = {:?} !", nick_player);
        if bot.pose_riddle(riddle.to_string(), answer.to_string(), &nick_player, timeout_ms).await {
            bot.try_expect(&format!("KICK #ApertureScience {:?}", nick_player), "KICK", "Failed to kick player", timeout_ms).await?;
        }
    }
    Ok(())
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let timeout_ms = 0;
    glados(timeout_ms).await?;
    Ok(())
}
