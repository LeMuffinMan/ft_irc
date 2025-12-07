use crate::client::Client;
use anyhow::Result;

pub async fn nick_normal_claim_and_change(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("nick_change_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            &format!("NICK {}_claimed\r\n", id),
            "updated his nickname to",
            "Failed to update nickname ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn nick_no_nickname_given(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("nonickgiven_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "NICK\r\n",
            "No nickname given",
            "Failed to reject update ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn nick_already_in_use(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client
        .try_expect(
            "PASS password\r\nNICK reserved_nick\r\n",
            "Nickname is already in use",
            "ERR_NICKALREADYINUSE missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}
