use crate::client::Client;
use anyhow::Result;
use tokio::time::{Duration, sleep};

pub async fn reserved_nick_client(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let nick = "reserved_nick".to_string();

    client.authenticate(nick, timeout_ms).await?;
    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

pub async fn no_mdp_chan_client(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let nick = "no_mdp_chan_client".to_string();

    client.authenticate(nick, timeout_ms).await?;
    client.send("JOIN #no_mdp_chan\r\n", 0).await?;

    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

pub async fn mdp_chan_client(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let nick = "mdp_chan_client".to_string();

    client.authenticate(nick, timeout_ms).await?;
    client.send("JOIN #mdp_chan\r\n", 0).await?;
    client.send("MODE #mdp_chan +k chan\r\n", 0).await?;

    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

pub async fn invite_chan_client(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let nick = "invite_chan_client".to_string();

    client.authenticate(nick, timeout_ms).await?;
    client.send("JOIN #invite_chan\r\n", 0).await?;
    client.send("MODE #invite_chan +i\r\n", 0).await?;

    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

pub async fn privmsg_client_chan(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let nick = "invite_chan_client".to_string();

    client.authenticate(nick, timeout_ms).await?;
    client.send("JOIN #privmsg_chan\r\n", 0).await?;

    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                } else {
                    //checker si chan / user -> repondre ne fct
                    let _ = client.send(&format!("{l}"), 0).await;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

pub async fn privmsg_client_nick(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let nick = "privmsg_nick_client".to_string();

    client.authenticate(nick, timeout_ms).await?;
    client.send("JOIN #privmsg_nick\r\n", 0).await?;

    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                } else {
                    //checker si chan / user -> repondre ne fct
                    let _ = client.send(&format!("{l}"), 0).await;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}
