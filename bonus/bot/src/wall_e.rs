use crate::Bot;

impl Bot {
    pub async fn wall_e_riddle(
        &mut self,
        riddle: &String,
        nick_player: &String,
        timeout_ms: u64,
    ) -> Result<bool, ()> {
        loop {
            if let Ok(_) = self.send(&riddle, timeout_ms).await {
                let _ = tokio::time::sleep(std::time::Duration::from_millis(200));
                if let Ok(Some(player_answer)) = self.read_line_timeout(timeout_ms).await {
                    println!("Wall-e : Player_answer = {player_answer}");

                    if player_answer.ends_with(":2\r\n") {
                        self.send(
                            &format!("PRIVMSG {nick_player} :Oh is that this simple ? *The robot plant a pizza and sees it works, a pizza tree juste grow, with pizzas as leaves and fruits
Thank you ! Here take a pizza, you might need it later ...\r\n"),
                            0
                        ).await.ok();
                        // let _ = self.send(&format!("PRIVMSG {nick_player} :Huh. There isn't enough neurotoxin to kill you. So I guess you win.\nTake this Aperture Science Handheld Portal Device, it does not make portal anymore but it translates roself languages\r\n"), timeout_ms).await;
                        println!("Pose riddle: Good answer");
                        return Ok(true);
                    } else if player_answer.ends_with(":1\r\n") {
                        println!("Pose riddle : Bad answer");
                        self.send(
                            &format!("KICK #BuyNLarge {nick_player} : Everyone will die as soon as they land back on earth because of you !!\r\n"),
                            0
                        ).await.ok();
                        return Ok(false);
                    } else {
                        println!("Pose riddle : Bad answer");
                        self.send(
                            &format!("KICK #ApertureScience {nick_player} : I hope others humans are smarter than you ...\r\n"),
                            0
                        ).await.ok();
                        return Ok(false);
                    }
                } else {
                    return Err(());
                }
            }
        }
    }
}

pub async fn wall_e(timeout_ms: u64) -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let mut bot = Bot::connect(6667).await?;
    let nick = "Wall-E";

    // let answer = "2";
    bot.authenticate(nick.to_string(), timeout_ms).await?;
    bot.try_expect(
        "JOIN #BuyNLarge\r\n",
        "Wall-E JOIN #BuyNLarge",
        "Wall-E failed to join channel",
        timeout_ms,
    )
    .await?;
    while let Some(line) = bot.read_line_timeout(timeout_ms).await? {
        println!("Wall-E received [{line}]");
        if line.starts_with(":GladOS") {
            if let Some(idx) = line.rfind(':') {
                let nick_player = &line[idx + 1..].trim();
                bot.try_expect(
                    &format!("INVITE {nick_player} #BuyNLarge\r\n"),
                    "341",
                    "Failed to invite user on #BuyNLarge",
                    timeout_ms,
                )
                .await?;
                let _ = tokio::time::sleep(tokio::time::Duration::from_secs(2)).await;
                let riddle = &format!("PRIVMSG {nick_player} :*The robot express itself only with robot noises, but somehow, the Aperture Science Handheld Portal Device translates it in real time :\n
Humanity will come back soon on earth and I didn't had time to clean everything!. 
Be usefull you lazy human, can you tell me how to make pizza to welcome them ?

[1] -> It's an old ancestral knowledge, no one knows anymore how to make pizza !
[2] -> You just need to plant some pizzas, then you can grow pizza trees, and have free pizza\r\n");
                match bot
                    .wall_e_riddle(riddle, &nick_player.to_string(), timeout_ms)
                    .await
                {
                    Ok(true) => {
                        bot.try_expect(
                            &format!("PRIVMSG Chat-GPT :{nick_player}\r\n"),
                            &format!("PRIVMSG Chat-GPT :{nick_player}"),
                            "Failed to send msg to Chat-GPT",
                            timeout_ms,
                        )
                        .await?;
                    }
                    _ => {
                        println!("Glados : wrong answer");
                    }
                }
                break;
            }
        } else if line.ends_with("JOIN #BuyNLarge\r\n") {
            if let Some(idx) = line.find(':') {
                let after_colon = &line[idx + 1..];
                let end_idx = after_colon.find(' ').unwrap_or(after_colon.len());
                let nick_player = &after_colon[..end_idx];
                bot.send(
                    &format!("PRIVMSG {nick_player} :Bip Booop bap bzz noise Clap Trap biiiiiip BOOM !\r\n"),
                    timeout_ms,
                )
                .await?;
                let _ = tokio::time::sleep(std::time::Duration::from_secs(2)).await;
                bot.try_expect(
                    &format!("KICK #BuyNLarge {nick_player}\r\n"),
                    &format!("KICK #BuyNLarge {nick_player}"),
                    "Failed to kick player",
                    timeout_ms,
                )
                .await?;
            }
        }
    }
    bot.shutdown().await?;
    Ok(())
}
