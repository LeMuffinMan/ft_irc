use crate::client::Client;
use anyhow::Result;

// PrivmsgCannotSendToChan,
// - NORMAL : 2 client in same chan
// - TOOMANYTARGET
// - Not registerd
// - NOT op in chan targeted ?
//

pub async fn privmsg_no_such_nick(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgnosuchnick", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.starts_with("PING") {
            let resp = line.replace("PING", "PONG");
            client.send(&resp, 0).await?;
        }
    }
    client
        .try_expect(
            "PRIVMSG nosuchnickname :msg\r\n",
            " :No such nick/channel",
            "ERR_NOSUCHNICK missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

// pub async fn privmsg_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
//     let channel = format!("stress_{}", id);
//     let mut client = Client::connect(port).await?;
//
//     client.send("PASS password\r\n", 0).await?;
//     client
//         .send(&format!("NICK {}_pmsg_ns_chan\r\n", channel), 0)
//         .await?;
//     client
//         .send(
//             &format!("USER {}_pmsg_ns_chan 0 * :pmsg_ns_chan\r\n", channel),
//             0,
//         )
//         .await?;
//
//     if let Some(line) = client.read_line_timeout(timeout_ms).await? {
//         if !line.contains("Welcome to the Internet Relay Network") {
//             return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
//         }
//     }
//     if let Some(line) = client.read_line_timeout(timeout_ms).await? {
//         if line.starts_with("PING") {
//             let resp = line.replace("PING", "PONG");
//             client.send(&resp, 0).await?;
//         }
//     }
//     client.send("PRIVMSG #nosuchchannel\r\n", 0).await?;
//     if let Some(line) = client.read_line_timeout(timeout_ms).await? {
//         if !line.contains(" :No such channel") {
//             return Err(anyhow::anyhow!("ERR_NOSUCHCHAN missing | received [{line}]"));
//         }
//     }
//     client.shutdown().await?;
//     Ok(())
// }

pub async fn privmsg_no_recipient(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgnorecipient", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    // if let Some(line) = client.read_line_timeout(timeout_ms).await? {
    //     if line.starts_with("PING") {
    //         let resp = line.replace("PING", "PONG");
    //         client.send(&resp, 0).await?;
    //     }
    // }
    client
        .try_expect(
            "PRIVMSG\r\n",
            " :No recipient given",
            "ERR_NORECIPIENT missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn privmsg_no_text_to_send(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}privmsgnotexttosend", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PRIVMSG target\r\n",
            " :No text to send",
            "ERR_NORECIPIENT missing ",
            timeout_ms,
        )
        .await?;
    // if let Some(line) = client.read_line_timeout(timeout_ms).await? {
    //     if line.starts_with("PING") {
    //         let resp = line.replace("PING", "PONG");
    //         client.send(&resp, 0).await?;
    //     }
    // }
    client.shutdown().await?;

    Ok(())
}
