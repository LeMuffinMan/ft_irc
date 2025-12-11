


use crate::Bot;

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
    loop {
        if let Some(line) = bot.read_line_timeout(timeout_ms).await? {
            if line.starts_with(":GladOS") {
                let player_name = line.rfind(':');
                bot.try_expect(
                    &format!("INVITE #BuyNLarge {:?}\r\n", player_name),
                    "341",
                    "Failed to invite user on #BuyNLarge",
                    timeout_ms,
                )
                .await?;
                break;
            } else if line.starts_with("JOIN") {
                let nick_player = line.rfind(':');
                bot.send(
                    "Bip Booop bap bzz noise Clap Trap biiiiiip BOOM !\r\n",
                    timeout_ms,
                )
                .await?;
                let _ = tokio::time::sleep(std::time::Duration::from_secs(3));
                bot.try_expect(
                    &format!("KICK #BuyNLarge {:?}\r\n", nick_player),
                    &format!("KICK #BuyNLarge {:?}", nick_player),
                    "Failed to kick player",
                    timeout_ms,
                )
                .await?;
            }
        }
    }
    loop {
        if let Some(nick_player) = bot.get_user_nick(timeout_ms).await {
            println!("nick_player = {:?} !", nick_player);
            let riddle = &format!("PRIVMSG {nick_player} :*The robot express itself only with robot noises, but somehow, the Aperture Science Handheld Portal Device translates it in real time :\n
        Humanity will come back soon on earth and I didn't had time to clean everything!. 
        Be usefull you lazy human, can you tell me how to make pizza to welcome them ?

        [1] -> It's an old ancestral knowledge, no one knows anymore how to make pizza !
        [2] -> You just need to plant some pizzas, then you can grow pizza trees, and have free pizza\r\n");
            if let Ok(result) = bot
                .pose_riddle(&riddle.to_string(), &nick_player, timeout_ms)
                .await
            {
                if result {
                    bot.try_expect(
                        &format!("PRIVMSG Chat-GPT :{nick_player}"),
                        "PRVIMSG ...",
                        "Failed to send msg to Chat-GPT",
                        timeout_ms,
                    )
                    .await?;
                }
                bot.try_expect(
                    &format!("KICK #BuyNLarge {nick_player}"),
                    "KICK #BuyNLarge {nick_player}",
                    "Failed to kick player",
                    timeout_ms,
                )
                .await?;
            }
        } else {
            bot.shutdown().await?;
            unreachable!("Failed posing riddle");
        }
    }
}
