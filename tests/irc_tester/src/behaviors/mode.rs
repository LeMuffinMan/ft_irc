use crate::client::Client;
use anyhow::Result;


/**
*
* @Brief Mode Command tests
*
*/

pub async fn mode_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_mode_more_params", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "MODE \r\n",
            " :Not enough parameters",
            "ERR_NEEDMOREPARAMS missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn mode_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client
        .try_expect(
            "MODE +i\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
