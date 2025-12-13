use crate::client::Client;
use anyhow::Result;

pub async fn part_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("partneedmorep_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PART \r\n",
            " :Not enough parameters",
            "ERR_NEEDMOREPARAMS missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn part_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client
        .try_expect(
            "PART #anychan\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
