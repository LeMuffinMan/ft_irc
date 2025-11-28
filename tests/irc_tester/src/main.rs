use rand::prelude::IndexedRandom;
use rand::rng;
use std::time::Instant;
use anyhow::Result;
use tokio::time::Duration;

mod client;
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
    let debug = false;

    //random noise
    // random_noise().await?;
    normal_connection(port, debug).await?;

    //faux mot passe
    normal_connection_wrong_password(port, debug).await?; //should disconnect immediately
    //not PASS as first command
    // normal_connection_not_pass_first(port, debug).await?;
    //Remplacer un nick
    // normal_connection_override_existing_nick(port).await?; //attente implementations
    //Remplacer un user
    // normal_connection_override_existing_user(port).await?; //attente implementations
    // SEGFAULT !
    // fragmented_messages(port, debug).await?; //Après X secondes sans \r\n → timeout
    low_bandwidth(port, debug).await?;

    //Overflow du buffer (> 512)
    // read_overflow(port).await?;   //On veut un MAX buffer sur la lecture ?
    //  max 512 octets par ligne
    //  max X lignes par seconde
    // Le serveur NE doit PAS :
    //  Bufferiser tout dans un énorme String.
    //  Faire un .push_str() sur un buffer qui explose.
    //  Bloquer la boucle événementielle (epoll/poll).
    //  Crasher/segfault/panique.
    //
    //UNICODES
    //Chars interdits :
    //NICK \xC0bad\r\n
    //USER test 0 * :name\xFF\r\n
    //
    // Timeout tests
    // - PONG kick
    // - not registered and only answer to ping
    // - partial registration and only answer to ping
    //
    //
    // -- parrallele --

    stress_test(6667, 1000, 0).await?;

    test_behaviors(6667, 20000).await?;
    advanced_stress_test(6667, 1000, 20000).await?;

    //- tester le non bloquant (Bible et normal connection en meme temps)
    //- Overflow 2 fd

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

fn ok(msg: &str) {
    println!("{} \x1b[32mOK\x1b[0m", msg);
}

fn ko(msg: &str) {
    println!("{} \x1b[31mKO\x1b[0m", msg);
}

async fn stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
    println!("Starting stress test with {} clients...", num_clients);

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
    let spawn_duration = start.elapsed();
    println!("All tasks spawned in: {:?}", spawn_duration);

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
    println!("Time per client: {:?}", total_duration / num_clients as u32);
    Ok(())
}

// async fn uncomplete_inactive_connection(port: u16, debug: bool) -> Result<()> {
//     let stream = TcpStream::connect(("127.0.0.1", port)).await?;
//     let (reader, mut writer) = stream.into_split();
//     let mut reader = BufReader::new(reader);
//     let mut line = String::new();
//
//     let auth_messages = [
//         "PASS password\r\n",
//         "NICK player1\r\n",
//     ];
//
//     println!("Expecting timeout after 10 secs ...");
//     for msg in &auth_messages {
//         if debug {
//             print!(">> {}", msg);
//         }
//         writer.write_all(msg.as_bytes()).await?;
//         sleep(Duration::from_millis(50)).await;
//     }
//     //we must be kicked in 10 secs
//     match tokio::time::timeout(Duration::from_millis(15000), reader.read_line(&mut line)).await {
//         Ok(Ok(n)) if n == 0 => {
//             ok("Uncomplete inactive connexion: ");
//         },
//         _ => {
//             ko("Uncomplete inactive connexion: ");
//         }
//     }
//     writer.shutdown().await?;
//     Ok(())
// }

//on deconnecte le client qui a renseigne un mauvais mot de passe ?
async fn normal_connection_wrong_password(port: u16, debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;

    let msg = "PASS incorrect_password\r\n";
    if debug {
        print!(">> {}", msg);
    }
    client.send(msg, 0).await?;


    let mut responses = Vec::new();
    loop {
        if let Some(line) = client.read_line_timeout(100).await? {
            if debug {
                print!("<< {}", line);
            }
            responses.push(line);
        } else {
            break;
        }
    }
    let combined = responses.concat();
    if combined.contains("Incorrect password") {
        ok("Wrong password:");
    } else {
        ko("Wrong password:");
    }
    client.shutdown().await?;
    Ok(())
}

async fn normal_connection(port: u16, debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let messages = [
        "PASS password\r\n",
        "NICK player1\r\n",
        "USER Jefferson 0 * :test user\r\n",
    ];

    for msg in messages {
        // print!(">> {}", msg);
        client.send(msg, 0).await?;
    }

    let mut responses = Vec::new();
    loop {
        if let Some(line) = client.read_line_timeout(100).await? {
            if debug {
                print!("<< {}", line);
            }
            responses.push(line);
        } else {
            break;
        }
    }
    let combined = responses.concat();
    if combined.contains("successfully registered") {
        ok("Simple connexion:");
    } else {
        ko("Simple connexion:");
    }

    //a deplacer
    // if line.starts_with("PING") {
    //     let payload = line.trim().trim_start_matches("PING").trim();
    //     let response = format!("PONG {}\r\n", payload);
    //     writer.write_all(response.as_bytes()).await?;
    //     println!(">> {}", response.trim());
    // }
    client.shutdown().await?;
    Ok(())
}

pub async fn fragmented_messages(port: u16, debug: bool) -> Result<()> {
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
        if debug {
            print!(">> {}", frag.escape_default());
        }
        client.send_raw(frag.as_bytes()).await?;
        tokio::time::sleep(Duration::from_millis(*delay)).await;
    }

    let mut responses = Vec::new();
    loop {
        if let Some(line) = client.read_line_timeout(100).await? {
            if debug {
                print!("<< {}", line);
            }
            responses.push(line.clone());

            // if line.starts_with("PING") {
            //     let payload = line.trim().trim_start_matches("PING").trim();
            //     let pong = format!("PONG {}\r\n", payload);
            //     client.send(&pong, 0).await?;
            //
            //     if debug {
            //         print!(">> {}", pong);
            //     }
            // }
        } else {
            break;
        }
    }

    let combined = responses.concat();
    if combined.contains("successfully registered") {
        ok("Fragmented auth:");
    } else {
        ko("Fragmented auth:");
    }

    client.shutdown().await?;
    Ok(())
}


pub async fn low_bandwidth(port: u16, debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let msg = "PASS password\r\nNICK slow\r\nUSER slow 0 * :slow user\r\n";

    if debug {
        println!("Sending char by char...");
    }

    for b in msg.bytes() {
        if debug {
            print!("{}", b as char);
        }
        client.send_raw(&[b]).await?;
        tokio::time::sleep(Duration::from_millis(10)).await;
    }

    tokio::time::sleep(Duration::from_millis(1000)).await;


    let mut responses = Vec::new();
    loop {
        if let Some(line) = client.read_line_timeout(200).await? {
            if debug {
                print!("<< {}", line);
            }
            responses.push(line.clone());

            // if line.starts_with("PING") {
            //     let payload = line.trim().trim_start_matches("PING").trim();
            //     let pong = format!("PONG {}\r\n", payload);
            //     client.send(&pong, 0).await?;
            //
            //     if debug {
            //         print!(">> {}", pong);
            //     }
            // }
        } else {
            break;
        }
    }

    let combined = responses.concat();
    if combined.contains("successfully registered") {
        ok("Low bandwidth:");
    } else {
        ko("Low bandwidth:");
    }

    client.shutdown().await?;
    Ok(())
}

async fn test_behaviors(port: u16, timeout_ms: u64) -> Result<()> {
    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::LegitIgnorePong,
        ClientBehavior::StartIgnoreAll,
        ClientBehavior::PongOnly,
        ClientBehavior::PongWithoutConnect,
    ];
    let mut res = Ok(());

    for behavior in behaviors {
        let result = run_client(port, 0, behavior, timeout_ms).await;
        match result {
            ClientResult { success: true, .. } => {
                println!("Behavior {:?} \x1b[32mOK\x1b[0m passed", behavior);
            }
            ClientResult { success: false, message, .. } => {
                println!("Behavior {:?} \x1b[31mKO\x1b[0m failed: {:?}", behavior, message);
                res = Err(anyhow::anyhow!(format!("Behavior {:?} \x1b[31mKO\x1b[0m failed: {:?}", behavior, message)));
            }
        }
    }
    return res;
}

async fn advanced_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {

    test_behaviors(port, timeout_ms).await?; 

    println!("Starting advanced stress test with {} clients...", num_clients);

    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::LegitIgnorePong,
        ClientBehavior::StartIgnoreAll,
        ClientBehavior::PongOnly,
        ClientBehavior::PongWithoutConnect,
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
                    eprintln!("Client {} [{:?}] failed: {:?}", client_result.id, client_result.behavior, client_result.message);
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

async fn run_client(port: u16, id: usize, behavior: ClientBehavior, timeout_ms: u64) -> ClientResult {
    let nick = format!("stress_{}", id);
    let mut client = match Client::connect(port).await {
        Ok(c) => c,
        Err(e) => return ClientResult { id, behavior, success: false, message: Some(format!("Connect failed: {}", e)) },
    };

    let result: anyhow::Result<()> = match behavior {
        ClientBehavior::LegitDisconnect => async {
            client.send("PASS password\r\n", 0).await?;
            client.send(&format!("NICK {}\r\n", nick), 0).await?;
            client.send(&format!("USER {} 0 * :LegitDisconnect\r\n", nick), 0).await?;

            if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                if !line.contains("successfully registered") {
                    return Err(anyhow::anyhow!("Welcome message missing"));
                }
            }
            client.shutdown().await?;
            Ok(())
        }.await,

        ClientBehavior::LegitIgnorePong => async {
            client.send("PASS password\r\n", 0).await?;
            client.send(&format!("NICK {}\r\n", nick), 0).await?;
            client.send(&format!("USER {} 0 * :LegitIgnorePong\r\n", nick), 0).await?;

            if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                if !line.contains("successfully registered") {
                    return Err(anyhow::anyhow!("Welcome message missing"));
                }
            }
            if let Some(kick) = client.read_line_timeout(timeout_ms).await? {
                if kick.contains("Disconnected: time out") {
                    return Ok(());
                }
            }
            return Err(anyhow::anyhow!("Expected a server kick"));
        }.await,

        ClientBehavior::StartIgnoreAll => async {
            client.send("PASS password\r\n", 0).await?;
            client.send(&format!("NICK {}_StartIgnoreAll\r\n", nick), 0).await?;

            if let Some(_) = client.read_line_timeout(timeout_ms).await? {
                return Err(anyhow::anyhow!("Should not be welcomed"));
            }

            if let Some(kick) = client.read_line_timeout(timeout_ms).await? {
                if kick.contains("Disconnected: time out") {
                    return Ok(());
                }
            }
            return Err(anyhow::anyhow!("Expected a server kick"));
        }.await,

        ClientBehavior::PongOnly => async {
            client.send("PASS password\r\n", 0).await?;
            client.send(&format!("NICK {}_Uncomplete_registered_pong\r\n", nick), 0).await?;
            loop {
                if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                    if line.starts_with("PING") {
                        let resp = line.replace("PING", "PONG");
                        client.send(&resp, 0).await?;
                    } else if line.contains("Disconnected: time out") {
                        return Ok(());
                    }
                } else {
                    break;
                }
            }
            return Err(anyhow::anyhow!("Should have been kicked"));
        }.await,

        ClientBehavior::PongWithoutConnect => async {
            loop {
                if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                    if line.starts_with("PING") {
                        let resp = line.replace("PING", "PONG");
                        client.send(&resp, 0).await?;
                    } else if line.contains("Disconnected: time out") {
                        return Ok(());
                    }
                } else {
                    break;
                }
            }
            return Err(anyhow::anyhow!("Should have been kicked"));
        }.await,
    };

    match result {
        Ok(_) => ClientResult { id, behavior, success: true, message: None },
        Err(e) => ClientResult { id, behavior, success: false, message: Some(format!("{}", e)) },
    }
}

