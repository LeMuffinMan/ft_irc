use crate::client::Client;
use anyhow::Result;



/**
*
* @Brief Topic connection tests
*
*/
pub async fn topic_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_topic_params", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "TOPIC \r\n",
            " :Not enough parameters",
            "ERR_NEEDMOREPARAMS missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn topic_not_on_chan(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_topic_not_on_chan", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "TOPIC #no_mdp_chan\r\n",
            " :You're not on that channel",
            "ERR_NOTONCHAN missing ",
            timeout_ms,
        )
        .await?;
    client.send("TOPIC #no_mdp_chan\r\n", 0).await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn topic_no_topic(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_topic_no_topic", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #no_mdp_chan\r\n",
            "JOIN #no_mdp_chan",
            "Failed to join #no_mdp_chan ",
            timeout_ms,
        )
        .await?;
    client
        .try_expect(
            "TOPIC #no_mdp_chan\r\n",
            " :No topic is set",
            "ERR_NOTOPIC missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn topic_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS password\r\n", 0).await?;
    client
        .try_expect(
            "TOPIC #anychan\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
