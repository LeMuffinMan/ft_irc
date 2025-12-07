use crate::client::Client;
use anyhow::Result;

//Rpl inviting

pub async fn invite_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("inv_needmore_p_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "INVITE \r\n",
            " :Not enough parameters",
            "Err_NEEDMOREPARAMS missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn invite_not_on_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("inv_notonchan_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "INVITE reserved_nick #no_mdp_chan\r\n",
            " :You're not on that channel",
            "ERR_NOTONCHANNEL missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn invite_no_such_nick(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("invnosuch_n_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #join_no_such_nick_test\r\n",
            "JOIN #join_no_such_nick_test",
            "Failed to join a valid chan",
            timeout_ms,
        )
        .await?;
    client
        .try_expect(
            "INVITE nosuchnickname #join_no_such_nick_test\r\n",
            " :No such nick/channel",
            "Err_NOSUCHNICK missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn invite_no_priv(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_invite_no_priv", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #join_no_priv\r\n",
            "JOIN #join_no_priv",
            "Failed to join a valid chan",
            timeout_ms,
        )
        .await?;
    client
        .try_expect(
            "INVITE reserved_nick #join_no_priv\r\n",
            " :You're not channel operator",
            "ERR_CHANOPRIVSNEED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn invite_not_registered(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client
        .try_expect(
            "PASS password\r\nINVITE reserved_nick #join_no_priv\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
