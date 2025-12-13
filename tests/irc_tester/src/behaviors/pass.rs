use crate::client::Client;
use anyhow::Result;



/**
*
* @Brief Pass Command tests
*
*/
pub async fn pass_already_registered(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("pass_already_registered_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PASS password\r\n",
            "Unauthorized command (already registered)",
            "ERR_ALREADYREGISTERED missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn pass_need_more_params(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client
        .try_expect(
            "PASS\r\n",
            " :Not enough parameters",
            "ERR_NEEDMOREPARAMS on Pass missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn normal_connection_wrong_password(port: u16, _debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client
        .try_expect(
            "PASS incorrect_password\r\n",
            "Password incorrect",
            "Expected 'Invalid password' in server response ",
            0,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn pass_not_first(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;
    client
        .try_expect(
            &format!("NICK {}\r\nPASS password\r\n", nick),
            "PASS must be first",
            "Should not be welcomed ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
