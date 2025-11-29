use anyhow::Result;
use futures::stream::{FuturesUnordered, StreamExt};
use rand::prelude::IndexedRandom;
use rand::rng;
use std::time::Instant;
use tokio::time::{Duration, timeout};

mod client;
use std::env;
use crate::client::Client;
use crate::client::ClientBehavior;

#[derive(Debug)]
struct ClientResult {
    id: usize,
    behavior: ClientBehavior,
    success: bool,
    message: Option<String>,
}

//proteger port 0 et les autres : -> 1024

//Benchmark map vs vector -> 1 client -> 1000 clients

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;
    let args: Vec<String> = env::args().collect();
    let num_clients: usize = args[1].parse().expect("Argument must be a positive integer");
    // let debug = false;

    //random noise
    // random_noise().await?;
    //Overflow du buffer (> 512)
    //  max 512 octets par ligne
    //UNICODES
    //  Chars interdits :
    //  NICK \xC0bad\r\n
    //  USER test 0 * :name\xFF\r\n

    connection_stress_test(port, num_clients, 0).await?;
    advanced_stress_test(port, num_clients, 0).await?;

    //NICK :
    //- Normal : claim a free nick
    //- Normal : change for a free nick
    //- Not registered
    //- ERR_NONNICKNAMEGIVEN
    //- ERR_ERRONEUSNICKNAME
    //- ERR_NICKNAMEINUSE
    //
    //PASS:
    //- Normal : correct password
    //- not as first cmd
    //- ERRPASSWORDMISMATCHED
    //- NEEDMOREPARAMS
    //- ALREADYREGISTERED
    //
    //USER:
    //- normal : claim a username
    //???
    //ALREADYREGISTERED
    //NEEDMOREPARAMS

    //
    //JOIN:
    //- normal : rejoindre un canal en le creant
    //- normal : rejoindre un canal existant, avec un user dedans
    //
    //          ERR_NEEDMOREPARAMS
    //          ERR_BANNEDFROMCHAN
    //          ERR_INVITEONLYCHAN
    //          ERR_BADCHANNELKEY
    //          ERR_CHANNELISFULL
    //          ERR_BADCHANMASK
    //          ERR_NOSUCHCHANNEL
    //          ERR_TOOMANYCHANNELS
    //          ERR_TOOMANYTARGETS
    //          ERR_UNAVAILRESOURCE
    //          RPL_TOPIC
    //- Not registered
    //
    //INVITE :
    // - Normal : 2 clients, one inviting in an chan, another not in this chan
    // - NEEDMOREPARAMS
    // - NOSUCHNICK
    // - NOTONCHANNEL
    // - USERONCHANNEL
    // - RPL_AWAY
    // - RPL_INVITING
    // - CHANOPRIVSNEEDED
    // - RPL AWAY
    // - RPL INVITING
    // - Not registered
    //
    //PRIVMSG :
    // - NORMAL : 2 client in same chan
    // - TOOMANYTARGET
    // - NORECIPIENT
    // - NO TEXTTOSEND
    // - Not registerd
    // - NOT op in chan targeted ?
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
    Ok(())
}

// fn ok(msg: &str) {
//     println!("{} \x1b[32mOK\x1b[0m", msg);
// }
//
// fn ko(msg: &str) {
//     println!("{} \x1b[31mKO\x1b[0m", msg);
// }

async fn connection_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
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
                format!("NICK {}\r\n", nick),
                format!("USER {} 0 * :stress user\r\n", nick),
            ];

            for msg in messages {
                client.send(&msg, 0).await?;
            }

            if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                if line.contains("successfully registered") {
                    return Ok(());
                }
            }
            Err(anyhow::anyhow!("Failed to register client"))
        });
        handles.push(handle);
    }
    // let spawn_duration = start.elapsed();
    // println!("All tasks spawned in: {:?}", spawn_duration);

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
        "Stress test finished: {} \x1b[32mOK\x1b[0m, {} \x1b[31mKO\x1b[0m",
        ok_count, ko_count
    );
    println!("Total time: {:?}", total_duration);
    println!("Time per client: {:?}\n", total_duration / num_clients as u32);
    Ok(())
}

//on deconnecte le client qui a renseigne un mauvais mot de passe ?
async fn normal_connection_wrong_password(port: u16, _debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS incorrect_password\r\n", 0).await?;

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(100).await? {
        responses.push(line);
    }

    let combined = responses.concat();

    client.shutdown().await?;

    if combined.contains("Invalid password") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(
            "Expected 'Invalid password' in server response"
        ))
    }
}

async fn normal_connection(port: u16, _debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let messages = [
        "PASS password\r\n",
        "NICK player1\r\n",
        "USER Jefferson 0 * :test user\r\n",
    ];

    for msg in messages {
        client.send(msg, 0).await?;
    }

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(100).await? {
        responses.push(line);
    }

    let combined = responses.concat();

    client.shutdown().await?;

    if combined.contains("successfully registered") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(
            "Expected 'successfully registered' in server response"
        ))
    }
}

pub async fn fragmented_messages(port: u16, _debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let fragments: &[(&str, u64)] = &[
        ("PASS pass", 100),
        ("\r\n", 100),
        ("word\r\n", 100),
        ("N", 50),
        ("ICK", 50),
        (" test\r\n", 100),
        ("USE", 80),
        ("R ", 80),
        ("player2 0 * ", 80),
        (":test user\r\n", 100),
    ];

    for (frag, delay) in fragments {
        client.send_raw(frag.as_bytes()).await?;
        tokio::time::sleep(Duration::from_millis(*delay)).await;
    }

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(100).await? {
        responses.push(line);
    }

    let combined = responses.concat();

    client.shutdown().await?;

    if combined.contains("successfully registered") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(
            "Expected 'successfully registered' in server response"
        ))
    }
}

pub async fn low_bandwidth(port: u16, _debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let msg = "PASS password\r\nNICK slow\r\nUSER slow 0 * :slow user\r\n";

    for b in msg.bytes() {
        client.send_raw(&[b]).await?;
        tokio::time::sleep(Duration::from_millis(10)).await;
    }

    tokio::time::sleep(Duration::from_millis(1000)).await;

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(200).await? {
        responses.push(line);
    }

    let combined = responses.concat();

    client.shutdown().await?;

    if combined.contains("successfully registered") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(
            "Expected 'successfully registered' in server response"
        ))
    }
}

async fn test_behaviors(port: u16, timeout_ms: u64) -> Result<()> {
    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::LegitIgnorePong,
        ClientBehavior::StartIgnoreAll,
        ClientBehavior::PongOnly,
        ClientBehavior::PongWithoutConnect,
        ClientBehavior::NormalConnection,
        ClientBehavior::WrongPassword,
        // ClientBehavior::FragmentedMessages,
        ClientBehavior::LowBandwidth,
    ];

    let mut futures: FuturesUnordered<_> = behaviors
        .into_iter()
        .map(|behavior| async move {
            let result = run_client(port, 0, behavior, timeout_ms).await;
            (behavior, result)
        })
        .collect();

    let mut res = Ok(());

    while let Some((behavior, result)) = futures.next().await {
        match result {
            ClientResult { success: true, .. } => {
                println!("Behavior {:?} \x1b[32mOK\x1b[0m", behavior);
            }
            ClientResult {
                success: false,
                message,
                ..
            } => {
                println!(
                    "Behavior {:?} \x1b[31mKO\x1b[0m failed: {:?}",
                    behavior, message
                );
                res = Err(anyhow::anyhow!(format!(
                    "Behavior {:?} \x1b[31mKO\x1b[0m: {:?}",
                    behavior, message
                )));
            }
        }
    }

    res
}

async fn advanced_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
    test_behaviors(port, timeout_ms).await?;

    println!(
        "\nStarting advanced stress test with {} clients...",
        num_clients
    );

    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::LegitIgnorePong,
        ClientBehavior::StartIgnoreAll,
        ClientBehavior::PongOnly,
        ClientBehavior::PongWithoutConnect,
        ClientBehavior::NormalConnection,
        ClientBehavior::WrongPassword,
        // ClientBehavior::FragmentedMessages,
        ClientBehavior::LowBandwidth,
    ];

    let mut handles = vec![];
    let mut rng = rng();

    for i in 0..num_clients {
        let behavior = *behaviors.choose(&mut rng).unwrap();
        handles.push(tokio::spawn(run_client(port, i, behavior, timeout_ms)));
    }

    let mut ok_count = 0;
    let mut ko_count = 0;

    for handle in handles {
        match handle.await {
            Ok(client_result) => {
                if client_result.success {
                    ok_count += 1;
                } else {
                    ko_count += 1;
                    eprintln!(
                        "Client {} [{:?}] failed: {:?}",
                        client_result.id, client_result.behavior, client_result.message
                    );
                }
            }
            Err(e) => {
                ko_count += 1;
                eprintln!("Client task panicked: {:?}", e);
            }
        }
    }

    println!(
        "Advanced Stress test finished: {} \x1b[32mOK\x1b[0m, {} \x1b[31mKO\x1b[0m",
        ok_count, ko_count
    );

    Ok(())
}

async fn run_client(
    port: u16,
    id: usize,
    behavior: ClientBehavior,
    timeout_ms: u64,
) -> ClientResult {
    let nick = format!("stress_{}", id);
    let mut client = match Client::connect(port).await {
        Ok(c) => c,
        Err(e) => {
            return ClientResult {
                id,
                behavior,
                success: false,
                message: Some(format!("Connect failed: {}", e)),
            };
        }
    };

    let result: anyhow::Result<()> = match behavior {
        ClientBehavior::LegitDisconnect => {
            async {
                client.send("PASS password\r\n", 0).await?;
                client.send(&format!("NICK {}\r\n", nick), 0).await?;
                client
                    .send(&format!("USER {} 0 * :LegitDisconnect\r\n", nick), 0)
                    .await?;

                if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                    if !line.contains("successfully registered") {
                        return Err(anyhow::anyhow!("Welcome message missing"));
                    }
                }
                client.shutdown().await?;
                Ok(())
            }
            .await
        }

        ClientBehavior::LegitIgnorePong => {
            async {
                client.send("PASS password\r\n", 0).await?;
                client.send(&format!("NICK {}\r\n", nick), 0).await?;
                client
                    .send(&format!("USER {} 0 * :LegitIgnorePong\r\n", nick), 0)
                    .await?;

                let total_timeout = Duration::from_secs(20);

                let result = timeout(total_timeout, async {
                    loop {
                        match client.read_line_timeout(timeout_ms).await? {
                            Some(line) => {
                                if line.starts_with("PING") {
                                    continue;
                                } else if line.contains("timed out") {
                                    return Ok::<(), anyhow::Error>(());
                                }
                            }
                            None => {}
                        }
                    }
                })
                .await;

                match result {
                    Ok(inner) => inner,
                    Err(_) => Err(anyhow::anyhow!("Timeout 20s waiting for server kick")),
                }
            }
            .await
        }

        ClientBehavior::StartIgnoreAll => {
            async {
                client.send("PASS password\r\n", 0).await?;
                client
                    .send(&format!("NICK {}_StartIgnoreAll\r\n", nick), 0)
                    .await?;

                let total_timeout = Duration::from_secs(20);

                let result = timeout(total_timeout, async {
                    loop {
                        match client.read_line_timeout(timeout_ms).await? {
                            Some(line) => {
                                if line.starts_with("PING") {
                                    continue;
                                } else if line.contains("timed out") {
                                    return Ok::<(), anyhow::Error>(());
                                }
                            }
                            None => {}
                        }
                    }
                })
                .await;

                match result {
                    Ok(inner) => inner,
                    Err(_) => Err(anyhow::anyhow!("Timeout 20s waiting for server kick")),
                }
            }
            .await
        }

        ClientBehavior::PongOnly => {
            async {
                client.send("PASS password\r\n", 0).await?;
                client
                    .send(&format!("NICK {}_Uncomplete_registered_pong\r\n", nick), 0)
                    .await?;
                loop {
                    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                        if line.starts_with("PING") {
                            let resp = line.replace("PING", "PONG");
                            client.send(&resp, 0).await?;
                        } else if line.contains("timed out") {
                            return Ok(());
                        }
                    } else {
                        break;
                    }
                }
                return Err(anyhow::anyhow!("Should have been kicked"));
            }
            .await
        }

        ClientBehavior::PongWithoutConnect => {
            async {
                loop {
                    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                        if line.starts_with("PING") {
                            let resp = line.replace("PING", "PONG");
                            client.send(&resp, 0).await?;
                        } else if line.contains("timed out") {
                            return Ok(());
                        }
                    } else {
                        break;
                    }
                }
                return Err(anyhow::anyhow!("Should have been kicked"));
            }
            .await
        }
        ClientBehavior::NormalConnection => normal_connection(port, false).await,
        ClientBehavior::WrongPassword => normal_connection_wrong_password(port, false).await,
        // ClientBehavior::FragmentedMessages => fragmented_messages(port, false).await,
        ClientBehavior::LowBandwidth => low_bandwidth(port, false).await,
    };

    match result {
        Ok(_) => ClientResult {
            id,
            behavior,
            success: true,
            message: None,
        },
        Err(e) => ClientResult {
            id,
            behavior,
            success: false,
            message: Some(format!("{}", e)),
        },
    }
}
