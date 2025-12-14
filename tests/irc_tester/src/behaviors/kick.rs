//! Kick command tests
use crate::client::Client;
use anyhow::Result;


/// Kick Command tests
pub async fn kick_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS password\r\n", 0).await?;
    client
        .try_expect(
            "KICK\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn kick_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("kickneedmorep_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "KICK \r\n",
            " :Not enough parameters",
            "ERR_NEEDMOREPARAMS missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn kick_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("Kick_no_such_chan_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "KICK #nosuchchannel user\r\n",
            " :No such channel",
            "ERR_NOSUCHCHAN missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn kick_priviledges(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick1 = format!("inv_kickpriv_{}-1", id);
    let nick2 = format!("inv_kickpriv_{}-2", id);
    let nick3 = format!("inv_kickpriv_{}-3", id);
    let mut client1 = Client::connect(port).await?;
    let mut client2 = Client::connect(port).await?;
    let mut client3 = Client::connect(port).await?;
    client1.authenticate(nick1.clone(), timeout_ms).await?;
    client2.authenticate(nick2.clone(), timeout_ms).await?;
    client3.authenticate(nick3.clone(), timeout_ms).await?;

    client1
        .try_expect(
            &format!("JOIN #{}chan,\r\n", &nick1),
            &format!("JOIN"),
            "C1 Failed to join new chan",
            timeout_ms,
        )
        .await?;
    client1
        .try_expect(
            &format!("INVITE {} #{}chan\r\n", &nick2, &nick1),
            "341",
            "C1 No invite confirmation received ",
            timeout_ms,
        )
        .await?;
    client2
        .expect("INVITE", "C2 Not invit received", timeout_ms)
        .await?;
    client2
        .try_expect(
            &format!("JOIN #{}chan\r\n", &nick1),
            "JOIN",
            "C2 Failed to join chan",
            timeout_ms,
        )
        .await?;
    client1
        .expect("JOIN", "C1 Not invit received", timeout_ms)
        .await?;
    client1
        .try_expect(
            &format!("INVITE {} #{}chan\r\n", &nick3, &nick1),
            "341",
            "C1 No invite confirmation received ",
            timeout_ms,
        )
        .await?;
    client3
        .expect("INVITE", "No INVITE received", timeout_ms)
        .await?;
    client3
        .try_expect(
            &format!("JOIN #{}chan\r\n", &nick1),
            "JOIN",
            "C3 Failed to join chan",
            timeout_ms,
        )
        .await?;
    client3
        .try_expect(
            &format!("KICK #{}chan {} : client3 kick client3\r\n", &nick1, &nick3),
            " :You're not channel operator",
            "C3 kicked himself ",
            timeout_ms,
        )
        .await?;
    client2
        .expect("JOIN", "No joined received", timeout_ms)
        .await?;
    client2
        .try_expect(
            &format!("KICK #{}chan {} : client2 kick client3\r\n", &nick1, &nick3),
            " :You're not channel operator",
            "C2 kicked client3 without priv ",
            timeout_ms,
        )
        .await?;
    client1
        .expect("JOIN", "No joined received", timeout_ms)
        .await?;
    client1
        .try_expect(
            &format!("KICK #{}chan {} : client1 kick client3\r\n", &nick1, &nick3),
            "KICK ",
            "C1 failed to kick client3 ",
            timeout_ms,
        )
        .await?;
    client1
        .try_expect(
            &format!("KICK #{}chan {} : client1 kick client1\r\n", &nick1, &nick1),
            &format!("KICK #{}chan {} : client1 kick client1", &nick1, &nick1),
            "C1 failed to kick himself",
            timeout_ms,
        )
        .await?;
    client2
        .expect("KICK ", "No kick message recieved", timeout_ms)
        .await?;
    client3
        .expect("KICK ", "No kick message recieved", timeout_ms)
        .await?;
    client1.shutdown().await?;
    client2.shutdown().await?;
    client3.shutdown().await?;
    Ok(())
}
