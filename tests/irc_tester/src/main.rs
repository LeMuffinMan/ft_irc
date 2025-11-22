use anyhow::Result;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
};

#[tokio::main]
async fn main() -> Result<()> {
    let port = 6667;

    let stream = TcpStream::connect(("127.0.0.1", port)).await?;
    println!("Simple connexion: OK");

    let mut reader = BufReader::new(stream);
    let mut line = String::new();

    loop {
        line.clear();
        let n = reader.read_line(&mut line).await?;

        if n == 0 {
            println!("Connexion fermée par le serveur.");
            break;
        }

        print!("<< {}", line);

        if line.starts_with("PING") {
            let payload = line.trim().trim_start_matches("PING").trim();
            let response = format!("PONG {}\r\n", payload);

            reader.get_mut().write_all(response.as_bytes()).await?;
            println!(">> {}", response.trim());
        }
    }

    Ok(())
}

