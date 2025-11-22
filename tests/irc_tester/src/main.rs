use anyhow::Result;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
    time::{sleep, Duration},
};

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;

    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    let pass_str = "PASS password\r\n";
    let nick_str = "NICK muff\r\n";
    let user_str = "USER muffin 0 * :oelleaum\r\n";

    let (reader, mut writer) = stream.into_split();
    let mut reader = BufReader::new(reader);
    let mut line = String::new();

    print!(">> {pass_str}");
    writer.write_all(pass_str.as_bytes()).await?;
    print!(">> {nick_str}");
    writer.write_all(nick_str.as_bytes()).await?;
    print!(">> {user_str}");
    writer.write_all(user_str.as_bytes()).await?;

    let n = reader.read_line(&mut line).await?;

    if n == 0 {
        println!("Connexion closed by server");
        return Ok(());
    }

    print!("<< {}", line);
    if line.contains("successfully registered")
    {
        println!("Simple connexion: OK");
    }

    loop {
        line.clear();
        let n = reader.read_line(&mut line).await?;

        if n == 0 {
            println!("Connexion closed by server");
            break;
        }

        print!("<< {}", line);

        if line.starts_with("PING") {
            let payload = line.trim().trim_start_matches("PING").trim();
            let response = format!("PONG {}\r\n", payload);
            sleep(Duration::from_millis(1000)).await;
            writer.write_all(response.as_bytes()).await?;
            println!(">> {}", response.trim());
        }
        break;
    }
    sleep(Duration::from_secs(1)).await;
    writer.shutdown().await?;

    Ok(())
}
