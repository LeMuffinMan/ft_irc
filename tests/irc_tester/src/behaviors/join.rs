use crate::client::Client;
use anyhow::Result;

/**
*
* @Brief Join Command tests
*
*/
pub async fn join_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_join_more_params", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN \r\n",
            " :Not enough parameters",
            "ERR_NEEDMOREPARAMS missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_no_such_chan", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN nosuchchannel\r\n",
            " :No such channel",
            "ERR_NOSUCHCHAN missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_new_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_join_new_chan", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #newchan\r\n",
            "join_new_chan JOIN #newchan",
            "No confirmation of chan creation",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS password\r\n", 0).await?;
    client
        .try_expect(
            "JOIN #newchan\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_existing_chan(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_join_exis_chan", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #newchan\r\n",
            "JOIN #newchan",
            "Failed to join an existing chan without pw ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_existing_multi_chan(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_join_multi", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick.clone(), timeout_ms).await?;
    client
        .try_expect(
            &format!("JOIN #newchan{},#secondnewchan{}\r\n", nick, nick),
            "JOIN #newchan",
            "Failed to join first chan ",
            timeout_ms,
        )
        .await?;
    client
        .expect(
            &format!("JOIN #secondnewchan{}", nick),
            "Failed to join second chan ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_existing_chan_mdp(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_join_mdp_chan", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #mdp_chan chan\r\n",
            "JOIN #mdp_chan",
            "Failed to join chan_with_mdp ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn join_invite_only_chan(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_join_inviteO_chan", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #invite_chan chan\r\n",
            " :Cannot join channel (+i)",
            "ERR_INVITEONLYCHAN missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
