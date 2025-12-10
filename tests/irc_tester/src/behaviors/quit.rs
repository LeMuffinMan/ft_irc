use crate::client::Client;
use anyhow::Result;

//verifier si on a le message avant de quit
//Apres le quit on devrait sortir de la boucle read ?

//on devrait pouvoir Quit sans params non ?
// pub async fn quit_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
//     let nick = format!("quitneedmorep_{}", id);
//     let mut client = Client::connect(port).await?;
//
//     client.authenticate(nick, timeout_ms).await?;
//     client
//         .try_expect(
//             "QUIT \r\n",
//             " :Not enough parameters",
//             "ERR_NEEDMOREPARAMS missing ",
//             timeout_ms,
//         )
//         .await?;
//     client.shutdown().await?;
//     Ok(())
// }
