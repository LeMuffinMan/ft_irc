mod bot;

use bot::Bot;

async fn glados(timeout_ms: u64) -> Result<(), Box<dyn std::error::Error>> {
    let mut bot = Bot::connect(6667).await?;
    let nick = "GladOS";
    let answer = "2";
    bot.authenticate(nick.to_string(), timeout_ms).await?;
    bot.try_expect("JOIN #ApertureScience\r\n",
        "GladOS JOIN #ApertureScience",
        "GladOS failed to join channel",
        timeout_ms).await?;
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
        if let Ok(result) = bot.pose_riddle(riddle.to_string(), answer.to_string(), &nick_player, timeout_ms).await {
            if result {
                //envoyer un message a un autre bot pour inviter le user dans un nouveau channel
            } else {
                //kick user 
            }
        } else {
            unreachable!("Failed posing riddle");
        }
    }
    return Ok(());
}

//Tant qu'il n'a pas un message de GladOS, on envoie des bip boup
//si on le recoit, on invite le joueur, puis on donne l'enigme
async fn wall_e(timeout_ms: u64) -> Result<(), Box<dyn std::error::Error>> {
    let mut bot = Bot::connect(6667).await?;
    let nick = "Wall-E";
    let answer = "2";
    bot.authenticate(nick.to_string(), timeout_ms).await?;
    bot.try_expect("JOIN #BuyNLarge\r\n",
        "Wall-E JOIN #BuyNLarge",
        "Wall-E failed to join channel",
        timeout_ms).await?;
    loop {
        if let Some(nick_player) = bot.get_user_nick(timeout_ms).await {
            println!("nick_player = {:?} !", nick_player);
        let riddle = &format!("PRIVMSG {nick_player} :*The robot express itself only with robot noises, but somehow, the Aperture Science Handheld Portal Device translate it in real time :\n
        Humanity will soon come back on earth and I didn't had time to clean everything!. 
        Be usefull you lazy human, can you tell me how to make pizza to welcome them ?\r\n");
        //C'est un savoir faire ancestral dont seuls les italiens ont le secret
        //Il suffit de planter des pizzas, pour faire pousser des arbres a pizza
        //Je ne sais pas non plus comment faire des pizzas ...
        if let Ok(result) = bot.pose_riddle(riddle.to_string(), answer.to_string(), &nick_player, timeout_ms).await {
            if result {
                //envoyer un message a un autre bot pour inviter le user dans un nouveau channel
            } else {
                //kick user 
            }
        } else {
            unreachable!("Failed posing riddle");
        }
    }
    return Ok(());
}

async fn chat_gpt(timeout_ms: u64) -> Result<(), Box<dyn std::error::Error>> {
    let mut bot = Bot::connect(6667).await?;
    let nick = "Chat-GPT";
    let answer = "2";
    bot.authenticate(nick.to_string(), timeout_ms).await?;
    bot.try_expect("JOIN #OpenAI\r\n",
        "GladOS JOIN #OpenAI",
        "GladOS failed to join channel",
        timeout_ms).await?;
    loop {
        if let Some(nick_player) = bot.get_user_nick(timeout_ms).await {
            println!("nick_player = {:?} !", nick_player);
        let riddle = &format!("PRIVMSG {nick_player} :Well done {nick_player} you succeed to answer all my minions. Now this is your ultimate challenge. You must mak    \r\n");
        if let Ok(result) = bot.pose_riddle(riddle.to_string(), answer.to_string(), &nick_player, timeout_ms).await {
            if result {
                //envoyer un message a un autre bot pour inviter le user dans un nouveau channel
            } else {
                //kick user 
            }
        } else {
            unreachable!("Failed posing riddle");
        }
    }
    return Ok(());
}


#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let timeout_ms = 0;
    glados(timeout_ms).await?;
    Ok(())
}
