use anyhow::Result;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
    time::{sleep, Duration},
};

//Redefinir regles PR pour Dev
//Faire dev depuis Classes
//Gerer conflits apres merge epol loop 

//Benchmark map vs vector -> 1 client -> 1000 clients

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;
    //Tester ports authorises / interdits / invalids

    //client qui arrive t s'en va sans rien faire 
    //client qui sest deco avant de finir de s'auth
    //client qui se connecte partiellement, qui repond au ping, mais qui reste la
    //faux mot passe
    //
    //Remplacer un nick / user
    normal_connection(port).await?;
    fragemented_messages(port).await?;
    test_low_bandwidth(port).await?;
    //plus de tests mono client
    //Overflow du buffer (bible)
    //
    //Des tests avec plusieurs clients
    //- tester 0 FD MAX
    //- Overflow 2 fd
    //
    //tester le max clients simultanes 
    //- Limite de perf 
    //- latence (sans local host ?)
    
    Ok(())
}

async fn normal_connection(port: u16) -> Result <()> {
    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();

    let auth_messages = [
        "PASS password\r\n",
        "NICK player1\r\n", 
        "USER player1 0 * :test user\r\n",
    ];

    for msg in &auth_messages {
        print!(">> {}", msg);
        writer.write_all(msg.as_bytes()).await?;
        sleep(Duration::from_millis(50)).await;
    }
    let n = reader.read_line(&mut line).await?;
    if n == 0 {
        println!("Connexion closed by server");
        return Ok(());
    }
    
    print!("<< {}", line);
    if line.contains("successfully registered") {
        println!("Simple connection: OK");
    }

    if line.starts_with("PING") {
        let payload = line.trim().trim_start_matches("PING").trim();
        let response = format!("PONG {}\r\n", payload);
        writer.write_all(response.as_bytes()).await?;
        println!(">> {}", response.trim());
    }

    writer.shutdown().await?;
    Ok(())
}

async fn fragemented_messages(port: u16) -> Result <()> {
    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();


    println!(">> PASS pass");
    writer.write_all(b"PASS pass").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(100)).await;

    println!(">> word\\r\\n");
    writer.write_all(b"word\r\n").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(100)).await;

    println!(">> N");
    writer.write_all(b"N").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(50)).await;

    println!(">> ICK");
    writer.write_all(b"ICK").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(50)).await;

    println!(">> test\\r\\n");
    writer.write_all(b" test\r\n").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(100)).await;

    println!(">> USE");
    writer.write_all(b"USE").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(80)).await;

    println!(">> R ");
    writer.write_all(b"R ").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(80)).await;

    println!(">> player2 0 * ");
    writer.write_all(b"player2 0 * ").await?;
    writer.flush().await?;
    sleep(Duration::from_millis(80)).await;

    println!(">> :test user\\r\\n");
    writer.write_all(b":test user\r\n").await?;
    writer.flush().await?;

    sleep(Duration::from_millis(500)).await;

    let mut auth_responses = Vec::new();
    loop {
        line.clear();
        match tokio::time::timeout(Duration::from_millis(100), reader.read_line(&mut line)).await {
            Ok(Ok(n)) if n > 0 => {
                print!("<< {}", line);
                auth_responses.push(line.clone());
                
                //Faire une fonction dediee pour gerer le ping pong 
                if line.starts_with("PING") {
                    let payload = line.trim().trim_start_matches("PING").trim();
                    let response = format!("PONG {}\r\n", payload);
                    writer.write_all(response.as_bytes()).await?;
                    println!(">> {}", response.trim());
                }
            }
            _ => break,
        }
    }

    let auth_combined = auth_responses.concat();
    if auth_combined.contains("successfully registered") {
        println!("Fragmented auth: OK");
    } else {
        println!("Fragmented auth : KO");
        println!("answer recieved: {:?}", auth_combined);
    }
    writer.shutdown().await?;
    Ok(())
}

//Envoyer char by char suffit a simuler une bande passante faible ?
async fn test_low_bandwidth(port: u16) -> Result<()> {
    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();

    let message = "PASS password\r\nNICK slow\r\nUSER slow 0 * :slow user\r\n";
    
    println!("Sending char by char...");
    for byte in message.as_bytes() {
        print!("{}", *byte as char);
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
                print!("<< {}", line);
                received_response = true;
                
                if line.starts_with("PING") {
                    let payload = line.trim().trim_start_matches("PING").trim();
                    let response = format!("PONG {}\r\n", payload);
                    writer.write_all(response.as_bytes()).await?;
                    println!(">> {}", response.trim());
                }
            }
            _ => break,
        }
    }

    if received_response {
        println!("Low bandwith: OK");
    } else {
        println!("Low bandwith: KO");
    }

    writer.shutdown().await?;
    Ok(())
}
