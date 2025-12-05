use crate::client::Client;
use anyhow::Result;

//- normal : rejoindre un canal en le creant
//- normal : rejoindre un canal existant, avec un user dedans
//
//      Waiting MODE 
//          ERR_INVITEONLYCHAN
//          ERR_BADCHANNELKEY
//          ERR_CHANNELISFULL
//          ERR_TOOMANYTARGETS
//          RPL_JOINED -> la coder nous meme
//          RPL_TOPIC
//
//          ERR_TOOMANYCHANNELS
//

pub async fn join_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_join_more_params\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_join_more_params 0 * :join_need_more_params\r\n",
                nick
            ),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("JOIN \r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!("Err_NEEDMOREPARAMS missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn join_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let channel = format!("no_such_chan_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_join_ns_chan\r\n", channel), 0)
        .await?;
    client
        .send(
            &format!("USER {}_join_ns_chan 0 * :join_ns_chan\r\n", channel),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("JOIN nosuchchannel\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No such channel") {
            return Err(anyhow::anyhow!("ERR_NOSUCHCHAN missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}


pub async fn join_new_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let channel = format!("{}_join_new_chan", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_join_new_chan\r\n", channel), 0)
        .await?;
    client
        .send(
            &format!("USER {} 0 * :join_new_chan\r\n", channel),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("JOIN #newchan\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("join_new_chan JOIN #newchan") {
            return Err(anyhow::anyhow!("No confirmation of chan creation | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}


