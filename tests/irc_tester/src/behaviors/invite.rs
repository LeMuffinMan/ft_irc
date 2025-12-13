use crate::client::Client;
use anyhow::Result;
/**
*
* @Brief Invite command tests
*
* This first test combime 3 clients executed sequencially
* it tests invite, mode and join command in specific situations
*
*/
pub async fn invite_mode_i_join(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick1 = format!("inv_normalchan_{}-1", id);
    let nick2 = format!("inv_normalchan_{}-2", id);
    let nick3 = format!("inv_normalchan_{}-3", id);
    let mut client1 = Client::connect(port).await?;
    let mut client2 = Client::connect(port).await?;
    let mut client3 = Client::connect(port).await?;
    client1.authenticate(nick1.clone(), timeout_ms).await?;
    client2.authenticate(nick2.clone(), timeout_ms).await?;
    client3.authenticate(nick3, timeout_ms).await?;
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
            &format!("MODE #{}chan +i\r\n", &nick1),
            &format!("324"),
            "C1 Mode didnt answered 324",
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
        .expect("INVITE", "No INVITE received", timeout_ms).await?;
    client3
        .try_expect(
            &format!("JOIN #{}chan\r\n", &nick1),
            " :Cannot join channel (+i)",
            "C3 ERR_INVITEONLYCHAN missing",
            timeout_ms,
        )
        .await?;
    client2
        .try_expect(
            &format!("INVITE inv_normalchan_{}-3 #{}chan", id, &nick1),
            ":You're not channel operator",
            "C2 ERR_NOPRIV missing",
            timeout_ms,
        )
        .await?;
    client1
        .try_expect(
            &format!("INVITE inv_normalchan_{}-3 #{}chan", id, &nick1),
            "341",
            "C1 No invite confirmation received ",
            timeout_ms,
        )
        .await?;
    client1.shutdown().await?;
    client2.shutdown().await?;
    client3.shutdown().await?;
    Ok(())
}

/**
*
* Each replies of invites are tested
*
*/
pub async fn invite_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("inv_needmore_p_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "INVITE \r\n",
            " :Not enough parameters",
            "Err_NEEDMOREPARAMS missing ",
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
            "ERR_NOTONCHANNEL missing ",
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

pub async fn invite_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
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
