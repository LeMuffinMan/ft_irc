//! Glados bot
//!
//! the GladOS bot is supposed to immediately send its riddle to player as soon as
//! a player join its channel
//! in case of failure, it kicks player and continue its loop waiting for a new JOIN
//! in case of success it congrats player and send player nickname to Wall-E to allow
//! player to progress in game
use crate::Bot;
use anyhow::Result;

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

    while let Some(line) = bot.read_line_timeout(timeout_ms).await? {
        if line.contains("JOIN") {
            if let Some(idx) = line.rfind(':') {
                let after_colon = &line[idx + 1..].trim();
                let end_idx = after_colon.find(' ').unwrap_or(after_colon.len());
                let nick_player = &after_colon[..end_idx];
                let riddle = &format!(
                    "Alright, listen carefully, because I’m only going to say this once.
You stand before two doors.
One leads to cake.
The other leads to a room full of neurotoxin gaz, and absolutely no cake.

Choose wisely, or don’t.
The outcome is equally informative.
For science.

You monster.

[1] -> The Cake door
[2] -> The neurotoxin gaz and absolutely no cake door.\r\n"
                );
                let _ = tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
                match bot
                    .glados_riddle(riddle, &nick_player.to_string(), timeout_ms)
                    .await
                {
                    Ok(true) => {
                        bot.try_expect(
                            &format!("PRIVMSG Wall-E :{nick_player}\r\n"),
                            &format!("PRIVMSG Wall-E :{nick_player}"),
                            "Failed to send msg to Wall-E",
                            timeout_ms,
                        )
                        .await?;
                    }
                    _ => {}
                };
            }
        }
    }
    Ok(())
}

impl Bot {
    pub async fn glados_riddle(
        &mut self,
        riddle: &String,
        nick_player: &String,
        timeout_ms: u64,
    ) -> Result<bool, anyhow::Error> {
        let _ = self
            .send_line_by_line(riddle, nick_player, timeout_ms)
            .await;

        let _ = tokio::time::sleep(std::time::Duration::from_millis(200)).await;
        if let Ok(Some(player_answer)) = self.read_line_timeout(timeout_ms).await {
            if player_answer.ends_with(":2\r\n") {
                self.send(
                        &format!("PRIVMSG {nick_player} :Huh. There isn't enough neurotoxin to kill you. So I guess you win.\r\n"),
                        0
                    ).await?;
                let _ = tokio::time::sleep(std::time::Duration::from_millis(1000)).await;
                self.send(
                        &format!("PRIVMSG {nick_player} :Take this Aperture Science Handheld Portal Device.\r\n"),
                        0
                    ).await?;
                let _ = tokio::time::sleep(std::time::Duration::from_millis(1000)).await;
                self.send(
                        &format!("PRIVMSG {nick_player} :It does not make portal anymore but it translates robot languages.\r\n"),
                        0
                    ).await?;
                let _ = tokio::time::sleep(std::time::Duration::from_millis(1000)).await;
                return Ok(true);
            } else if player_answer.ends_with(":1\r\n") {
                self.try_expect(
                    &format!("KICK #ApertureScience {nick_player} : The Enrichment Center is required to remind you that you will be baked, and then there will be cake.\r\n"),
                    &format!("KICK #ApertureScience {nick_player}"),
                    "Failed to kick player",
                    timeout_ms,
                ).await?;
                return Ok(false);
            } else {
                self.try_expect(
                    &format!(
                        "KICK #ApertureScience {nick_player} : You are just smart as you seem\r\n"
                    ),
                    &format!("KICK #ApertureScience {nick_player}"),
                    "Failed to kick player",
                    timeout_ms,
                )
                .await?;
                return Ok(false);
            }
        }
        return Err(anyhow::anyhow!("Read returned None"));
    }
}
