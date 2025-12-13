use crate::client::Client;
use anyhow::Result;

/**
*
* @Brief User connection tests
*
*/
pub async fn user_already_registered(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("user_already_registered_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            &format!("USER {}_no_nick 0 * :NoNick\r\n", id),
            "Unauthorized command (already registered)",
            "ERR_ALREADYREGISTERED missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn user_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("user_more_p_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_no_nick\r\n", nick), 0)
        .await?;
    client
        .send(&format!("USER {}_no_nick 0 *\r\n", nick), 0)
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!(
                "ERR_NEEDMOREPARAMS on User missing | received [{line}]"
            ));
        }
    }
    client.shutdown().await?;
    Ok(())
}
