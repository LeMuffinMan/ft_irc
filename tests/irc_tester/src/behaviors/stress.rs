use crate::client::Client;
use anyhow::Result;

pub async fn continuous_noise(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let mut counter = 0;

    loop {
        let msg = format!("NOISE_{}\r\n", counter);
        counter += 1;

        client.send(&msg, 0).await?;
        tokio::time::sleep(tokio::time::Duration::from_millis(50)).await;

        if let Some(line) = client.read_line_timeout(timeout_ms).await? {
            if line.contains("timed out") {
                return Ok(());
            }
        } else {
            break;
        }
    }

    Err(anyhow::anyhow!("Server should have kicked us"))
}
