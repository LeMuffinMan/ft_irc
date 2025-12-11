
use crate::Bot;

pub async fn glados(timeout_ms: u64) -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let mut bot = Bot::connect(6667).await?;
    let nick = "GladOS";
    bot.authenticate(nick.to_string(), timeout_ms).await?;
    bot.try_expect(
        "JOIN #ApertureScience\r\n",
        "GladOS JOIN #ApertureScience",
        "GladOS failed to join channel",
        timeout_ms,
    )
    .await?;
    loop {
        if let Some(nick_player) = bot.get_user_nick(timeout_ms).await {
            println!("nick_player = {:?} !", nick_player);
            let riddle = &format!("PRIVMSG {nick_player} :Alright, listen carefully, because I’m only going to say this once.
        You stand before two doors.
        One leads to cake.
        The other leads to a room full of neurotoxin gaz, and absolutely no cake.

        Choose wisely, or don’t.
        The outcome is equally informative.
        For science.

        You monster.

        [1] -> The Cake door
        [2] -> The neurotoxin gaz and absolutely no cake door.
        \r\n");
            match bot.pose_riddle(riddle, &nick_player, timeout_ms).await {
                Ok(true) => {
                    bot.try_expect(
                        &format!("PRIVMSG Wall-E :{nick_player}"),
                        &format!("PRVIMSG Wall-E :{nick_player}"),
                        "Failed to send msg to Wall-E",
                        timeout_ms,
                    )
                    .await?;
                },
                _ => {
                    println!("Le joueur a répondu incorrectement.");
                },
            };
            bot.try_expect(
                &format!("KICK #ApertureScience {nick_player}"),
                "KICK #ApertureScience {nick_player}",
                "Failed to kick player",
                timeout_ms,
            ).await?;
        }
    }
}
