use crate::client::Client;
use anyhow::Result;

// ERR_NOTONCHANNEL
// RPL_NOTOPIC
// RPL_TOPIC
// ERR_CHANOPRIVSNEEDED
// ERR_NOCHANMODES

pub async fn topic_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_topic_more_params\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_topic_more_params 0 * :topic_need_more_params\r\n",
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

    client.send("TOPIC \r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!("Err_NEEDMOREPARAMS missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}
