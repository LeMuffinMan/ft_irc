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

    if stress_mode == 0 {
        let _ = test_behaviors(port, 0).await;
    }
    if beh_mode == 0 {
        // let connection_stress_handle = tokio::spawn(connection_stress_test(port, num_clients, 0));
        connection_stress_test(port, num_clients, timeout).await?;
        advanced_stress_test(port, num_clients, timeout).await?;
    }

    controle_handle.abort();
    no_mdp_chan_handle.abort();
    mdp_chan_handle.abort();
    invite_chan_handle.abort();

    Ok(())
}

//Todo
//
//INVITE :
// - Normal : 2 clients, one inviting in an chan, another not in this chan
// - RPL_INVITING
// - CHANOPRIVSNEEDED
// - Not registered
//

//
//KICK :
//- Normal : kick user
//- Normal : kick user :msg
//- Normal : chan user
//- Normal : chan user :msg
//- Normal : chan chan ... user user ...
//- Normal : chan chan ... user user ... :msg
//- @ -> as not operator
//- NEEDMOREPARAMS
//- BADCHANMASK
//- NO SUCH CHAN
//- USERNOTONCHAN
//- USERNOTINCHAN
//- Not registered
//-
// MODE:
// - ERR_NEEDMOREPARAMS
// - ERR_USERSDONTMATCH
// - ERR_UMODEUNKNOWNFLAG
// -RPL_UMODEIS
//
// PART:
// - ERR_NEEDMOREPARAMS
// - ERR_NOSUCHCHANNEL
// - ERR_NOTONCHANNEL
//
// QUIT:
// - quitter un chan commun et checker le message en face
