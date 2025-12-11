
use crate::Bot;

impl Bot {
    pub async fn glados_riddle(
        &mut self,
        riddle: &String,
        nick_player: &String,
        timeout_ms: u64,
    ) -> Result<bool, ()> {
        if let Ok(_) = self.send(&riddle, timeout_ms).await {
            let _ = tokio::time::sleep(std::time::Duration::from_millis(200));
            if let Ok(Some(player_answer)) = self.read_line_timeout(timeout_ms).await {
                println!("Player_answer = {player_answer}");
                
                if player_answer.ends_with(":2\r\n") {
                    self.send(
                        &format!("PRIVMSG {nick_player} :Huh. There isn't enough neurotoxin to kill you. So I guess you win.\nTake this Aperture Science Handheld Portal Device, it does not make portal anymore but it translates robot languages\r\n"),
                        0
                    ).await.ok();
                    // let _ = self.send(&format!("PRIVMSG {nick_player} :Huh. There isn't enough neurotoxin to kill you. So I guess you win.\nTake this Aperture Science Handheld Portal Device, it does not make portal anymore but it translates roself languages\r\n"), timeout_ms).await;
                    println!("Pose riddle: Good answer");
                    return Ok(true);
                } else if player_answer.ends_with(":1\r\n") {
                    println!("Pose riddle : Bad answer");
                    self.send(
                        &format!("KICK #ApertureScience {nick_player} : The Enrichment Center is required to remind you that you will be baked, and then there will be cake.\r\n"),
                        0
                    ).await.ok();
                    return Ok(false);
                } else {
                    println!("Pose riddle : Bad answer");
                    self.send(
                        &format!("KICK #ApertureScience {nick_player} : You are just smart as you seem\r\n"),
                        0
                    ).await.ok();
                    return Ok(false);
                }
            } else {
                return Err(());
            }
        }
        Err(())
    }
}

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
            match bot.glados_riddle(riddle, &nick_player, timeout_ms).await {
                Ok(true) => {
                    bot.try_expect(
                        &format!("PRIVMSG Wall-E :{nick_player}\r\n"),
                        &format!("PRIVMSG Wall-E :{nick_player}"),
                        "Failed to send msg to Wall-E",
                        timeout_ms,
                    )
                    .await?;
                },
                _ => {
                    println!("Glados : wrong answer");
                },
            };
        }
    }
}
