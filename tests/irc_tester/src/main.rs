mod client;
use crate::client::Client;

use anyhow::Result;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
    time::{sleep, Duration},
};

//Redefinir regles PR pour Dev

//proteger port 0 et les autres : -> 1024 
// 65535  

//Benchmark map vs vector -> 1 client -> 1000 clients
//
//

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;
    let debug = false;

    normal_connection(port, debug).await?;
    //normal connection et tester 2 pings

    //faux mot passe
    normal_connection_wrong_password(port, debug).await?;

    //client qui arrive t s'en va sans rien faire : quel comportement chercher ? on clean ? 
    // unactive_connection(port).await?;
    //client qui sest deco avant de finir de s'auth : idem
    // uncomplete_connection(port).await?;
    //
    //Remplacer un nick 
    // normal_connection_override_existing_nick(port).await?; //attente implementations
    //Remplacer un user
    // normal_connection_override_existing_user(port).await?; //attente implementations
    fragemented_messages(port, debug).await?; //Après X secondes sans \r\n → timeout
    low_bandwidth(port, debug).await?;

    //client qui se connecte partiellement, qui repond au ping, mais qui reste la : timeout de 10s
    // uncomplete_inactive_connection(port, debug).await?;

    //PASS apres NICK -> Doit echouer ? 
    //Overflow du buffer (bible)
    // read_bible(port).await?;   //On veut un MAX buffer sur la lecture ? 
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
    //Des tests avec plusieurs clients
    //- tester 0 -> FD MAX
    //- tester le non bloquant (Bible et normal connection en meme temps)
    //- Overflow 2 fd
    //
    //tester le max clients simultanes 
    //- Limite de perf 
    //- latence (sans local host ?)
    
    // -- parralle --

    stress_test(6667, 1100).await?;

    //faire un mode verbose !!
    // let tests: Vec<(&str, tokio::task::JoinHandle<Result<()>>)> = vec![
    //     ("normal_connection", tokio::spawn(normal_connection(port))),
    //     ("normal_connection_wrong_password", tokio::spawn(normal_connection_wrong_password(port))),
    //     ("fragemented_messages", tokio::spawn(fragemented_messages(port))),
    //     ("low_bandwidth", tokio::spawn(low_bandwidth(port))),
    //     ("uncomplete_inactive_connection", tokio::spawn(uncomplete_inactive_connection(port))),
    // ];
    //
    // for (name, handle) in tests {
    //     match handle.await {
    //         Ok(Ok(_)) => println!("{} \x1b[32mOK\x1b[0m", name),
    //         Ok(Err(e)) => println!("{} \x1b[31mKO\x1b[0m - {:?}", name, e),
    //         Err(e) => println!("{} \x1b[31mKO\x1b[0m - Task panicked: {:?}", name, e),
    //     }
    // }

    Ok(())
}

fn ok(msg: &str) {
    println!("{} \x1b[32mOK\x1b[0m", msg);
}

fn ko(msg: &str) {
    println!("{} \x1b[31mKO\x1b[0m", msg);
}

async fn stress_test(port: u16, num_clients: usize) -> Result<()> {
    println!("Starting stress test with {} clients...", num_clients);

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

            if let Some(line) = client.read_line_timeout(100).await? {
                if line.contains("successfully registered") {
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

    println!("Stress test finished: {} \x1b[32mOK\x1b[0m, {} \x1b[31mKO\x1b[0m", ok_count, ko_count);
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
    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();

    let auth_messages = [
        "PASS incorrect_password\r\n",
    ];

    for msg in &auth_messages {
        if debug {
            print!(">> {}", msg);
        }
        writer.write_all(msg.as_bytes()).await?;
        sleep(Duration::from_millis(50)).await;
    }

    loop {
        line.clear();
        match tokio::time::timeout(Duration::from_millis(200), reader.read_line(&mut line)).await {
            Ok(Ok(n)) if n > 0 => {
                if debug {
                    print!("<< {}", line);
                }
                if line.contains("Incorrect password")
                {
                    ok("Wrong password: ");
                }
                else
                {
                    ko("Wrong password: ");
                }
            }
            _ => {
                ko("Wrong password: ");
                break;
            }
        }
    }
    writer.shutdown().await?;
    Ok(())
}

async fn normal_connection(port: u16, debug: bool) -> Result <()> {
    let mut client = Client::connect(port).await?;
    let messages = [
        "PASS password\r\n",
        "NICK player1\r\n", 
        "USER player1 0 * :test user\r\n",
    ];

    for msg in messages {
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

async fn fragemented_messages(port: u16, debug: bool) -> Result <()> {
    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();

    if debug {
        println!(">> PASS pass");
    }
    writer.write_all(b"PASS pass").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(100)).await;

    if debug {
        println!(">> word\\r\\n");
    }
    writer.write_all(b"word\r\n").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(100)).await;

    if debug {
        println!(">> N");
    }
    writer.write_all(b"N").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(50)).await;

    if debug {
        println!(">> ICK");
    }
    writer.write_all(b"ICK").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(50)).await;

    if debug {
        println!(">> test\\r\\n");
    }
    writer.write_all(b" test\r\n").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(100)).await;

    if debug {
        println!(">> USE");
    }
    writer.write_all(b"USE").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(80)).await;

    if debug {
        println!(">> R ");
    }
    writer.write_all(b"R ").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(80)).await;

    if debug {
        println!(">> player2 0 * ");
    }
    writer.write_all(b"player2 0 * ").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(80)).await;

    if debug {
        println!(">> :test user\\r\\n");
    }
    writer.write_all(b":test user\r\n").await?;
    writer.flush().await?;

    sleep(Duration::from_millis(500)).await;

    let mut auth_responses = Vec::new();
    loop {
        line.clear();
        match tokio::time::timeout(Duration::from_millis(100), reader.read_line(&mut line)).await {
            Ok(Ok(n)) if n > 0 => {
                if debug {
                    print!("<< {}", line);
                }
                auth_responses.push(line.clone());
                
                //Faire une fonction dediee pour gerer le ping pong 
                if line.starts_with("PING") {
                    let payload = line.trim().trim_start_matches("PING").trim();
                    let response = format!("PONG {}\r\n", payload);
                    writer.write_all(response.as_bytes()).await?;
                    if debug {
                        println!(">> {}", response.trim());
                    }
                }
            }
            _ => break,
        }
    }

    let auth_combined = auth_responses.concat();
    if auth_combined.contains("successfully registered") {
        ok("Fragmented auth: ");
    } else {
        ko("Fragmented auth: ");
        // println!("answer recieved: {:?}", auth_combined);
    }
    writer.shutdown().await?;
    Ok(())
}

//Envoyer char by char a 10 ms suffit a simuler une bande passante faible ?
async fn low_bandwidth(port: u16, debug: bool) -> Result<()> {
    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();

    let message = "PASS password\r\nNICK slow\r\nUSER slow 0 * :slow user\r\n";
    
    if debug {
        println!("Sending char by char...");
    }
    for byte in message.as_bytes() {
        if debug {
            print!("{}", *byte as char);
        }
        writer.write_all(&[*byte]).await?;
        writer.flush().await?;
        sleep(Duration::from_millis(10)).await;
    }

    sleep(Duration::from_millis(1000)).await;
    
    let mut received_response = false;
    loop {
        line.clear();
        match tokio::time::timeout(Duration::from_millis(200), reader.read_line(&mut line)).await {
            Ok(Ok(n)) if n > 0 => {
                if debug {
                    print!("<< {}", line);
                }
                received_response = true;
                
                if line.starts_with("PING") {
                    let payload = line.trim().trim_start_matches("PING").trim();
                    let response = format!("PONG {}\r\n", payload);
                    writer.write_all(response.as_bytes()).await?;
                    if debug {
                        println!(">> {}", response.trim());
                    }
                }
            }
            _ => break,
        }
    }

    if received_response {
        ok("Low bandwith: ");
    } else {
        ko("Low bandwith: ");
    }

    writer.shutdown().await?;
    Ok(())
}
