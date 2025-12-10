use crate::client::Client;
use anyhow::Result;

//Error pour un parametre en plus
//checker si on recoit un timestamp
//

 // +00:00
 //
 
pub async fn time_with_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("modeneedmorep_{}", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "TIME now\r\n",
            "Time command does not take any parameters",
            "Time should not take any params ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn time_check_answer(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("timecheckanswer_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "TIME\r\n",
            "00:00",
            "Time answer not include time zone ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn time_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS password\r\n", 0).await?;
    client
        .try_expect(
            "TIME\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}
