use anyhow::Result;
use std::env;
mod behaviors;
mod client;
mod tests;
mod utils;
use crate::tests::advanced_test::test_behaviors;
use crate::tests::witness::witness_client;
use std::sync::Arc;
use tests::{advanced_stress_test, connection_stress_test};
use tokio::sync::Notify;

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;
    let args: Vec<String> = env::args().collect();
    let num_clients: usize = args[1]
        .parse()
        .expect("Argument must be a positive integer");
    let mode: usize = args[2]
        .parse()
        .expect("Argument must be a positive integer");

    let stop_signal = Arc::new(Notify::new());
    let witness_handle = tokio::spawn(witness_client(port, stop_signal.clone()));
    if mode == 0 {
        let _ = test_behaviors(port, 0).await;
    }
    let connection_stress_handle = tokio::spawn(connection_stress_test(port, num_clients, 0));
    connection_stress_handle.await??;
    advanced_stress_test(port, num_clients, 0).await?;

    stop_signal.notify_one();
    witness_handle.await??;

    Ok(())
}

//Todo
//

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
