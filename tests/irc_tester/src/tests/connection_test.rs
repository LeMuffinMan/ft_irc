use crate::client::Client;
use anyhow::Result;
use std::time::Instant;

pub async fn connection_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
    println!(
        "Starting connection stress test with {} clients...",
        num_clients
    );

    let start = Instant::now();
    let mut handles = Vec::with_capacity(num_clients);

    for i in 0..num_clients {
        let handle = tokio::spawn(async move {
            let nick = format!("stress_{}", i);
            let mut client = match Client::connect(port).await {
                Ok(c) => c,
                Err(_) => return Err(anyhow::anyhow!("Failed to connect")),
            };

            let messages = vec![
                "PASS password\r\n".to_string(),
                format!("NICK {}_stress_connection\r\n", nick),
                format!("USER {} 0 * :stress user\r\n", nick),
            ];

            for msg in messages {
                client.send(&msg, 0).await?;
            }

            if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                if line.contains("Welcome to the Internet Relay Network") {
                    return Ok(());
                }
            }
            Err(anyhow::anyhow!("Failed to register client"))
        });
        handles.push(handle);
    }

    let mut ok_count = 0;
    let mut ko_count = 0;

    for handle in handles {
        match handle.await {
            Ok(Ok(_)) => ok_count += 1,
            _ => ko_count += 1,
        }
    }

    let total_duration = start.elapsed();
    println!(
        "\nConnection stress test finished: {} \x1b[32mOK\x1b[0m, {} \x1b[31mKO\x1b[0m",
        ok_count, ko_count
    );
    println!("Total time: {:?}", total_duration);
    println!(
        "Time per client: {:?}\n",
        total_duration / num_clients as u32
    );
    Ok(())
}
