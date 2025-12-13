mod behavior;
mod behaviors;
mod client;
mod controle_clients;
mod result;
mod stress_tests;

use crate::controle_clients::*;
use crate::stress_tests::*;
use anyhow::Result;
use std::env;

fn parse() -> (usize, usize, usize) {
    let args: Vec<String> = env::args().collect();
    let num_clients: usize = args[1].parse().expect("Argument must be 0 or 1");
    let stress_mode: usize = args[2].parse().expect("Argument must be 0 or 1");
    let beh_mode: usize = args[3].parse().expect("Argument must be 0 or 1");

    (num_clients, stress_mode, beh_mode)
}

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;
    let timeout = 0;
    let (num_clients, stress_mode, beh_mode) = parse();

    let controle_handle = tokio::spawn(reserved_nick_client(port, timeout));
    let no_mdp_chan_handle = tokio::spawn(no_mdp_chan_client(port, timeout));
    let mdp_chan_handle = tokio::spawn(mdp_chan_client(port, timeout));
    let invite_chan_handle = tokio::spawn(invite_chan_client(port, timeout));
    let privmsg_client_nick_handle = tokio::spawn(privmsg_client_nick(port, timeout));
    let privmsg_client_chan_handle = tokio::spawn(privmsg_client_chan(port, timeout));

    if stress_mode == 0 {
        let _ = test_behaviors(port, 0).await;
    }
    if beh_mode == 0 {
        connection_stress_test(port, num_clients, timeout).await?;
        advanced_stress_test(port, num_clients, timeout).await?;
    }

    controle_handle.abort();
    no_mdp_chan_handle.abort();
    mdp_chan_handle.abort();
    invite_chan_handle.abort();
    privmsg_client_chan_handle.abort();
    privmsg_client_nick_handle.abort();

    Ok(())
}

