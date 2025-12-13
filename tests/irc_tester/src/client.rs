use anyhow::Result;
use std::sync::Arc;
use tokio::sync::Mutex;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader, BufWriter},
    net::TcpStream,
    time::{Duration, sleep},
};

pub struct Client {
    writer: Arc<Mutex<BufWriter<tokio::net::tcp::OwnedWriteHalf>>>,
    reader: tokio::io::BufReader<tokio::net::tcp::OwnedReadHalf>,
}

impl Client {
    pub async fn try_expect(
        &mut self,
        cmd: &str,
        expect: &str,
        error: &str,
        timeout_ms: u64,
    ) -> Result<()> {
        self.send(cmd, 0).await?;
        if let Some(line) = self.read_line_timeout(timeout_ms).await? {
            if line.starts_with("PING") {
                let resp = line.replace("PING", "PONG");
                self.send(&resp, 0).await?;
            } else if !line.contains(expect) {
                return Err(anyhow::anyhow!("{} | Received [{}]", error, line));
            }
        }
        Ok(())
    }

    pub async fn expect(&mut self, expect: &str, error: &str, timeout_ms: u64) -> Result<()> {
        while let Some(line) = self.read_line_timeout(timeout_ms).await? {
            if line.starts_with("PING") {
                let resp = line.replace("PING", "PONG");
                self.send(&resp, 0).await?;
            } else if !line.contains(expect) {
                return Err(anyhow::anyhow!("{} | Received [{}]", error, line));
            } else {
                break;
            }
        }
        Ok(())
    }

    pub async fn authenticate(&mut self, nick: String, timeout_ms: u64) -> Result<()> {
        self.send("PASS password\r\n", 0).await?;
        self.send(&format!("NICK {}\r\n", nick), 0).await?;
        self.send(
            &format!("USER {}_username 0 * :{}_username\r\n", nick, nick),
            0,
        )
        .await?;

        if let Some(line) = self.read_line_timeout(timeout_ms).await? {
            if !line.contains("Welcome to the Internet Relay Network") {
                return Err(anyhow::anyhow!(
                    "Welcome message missing | received [{line}]"
                ));
            }
        }
        Ok(())
    }

    pub async fn connect(port: u16) -> Result<Self> {
        let stream = TcpStream::connect(("127.0.0.1", port)).await?;
        let (reader, writer) = stream.into_split();
        Ok(Self {
            reader: BufReader::new(reader),
            writer: Arc::new(Mutex::new(BufWriter::new(writer))),
        })
    }

    pub async fn send(&self, msg: &str, delay_ms: u64) -> Result<()> {
        let mut writer = self.writer.lock().await;
        writer.write_all(msg.as_bytes()).await?;
        writer.flush().await?;
        drop(writer);
        if delay_ms > 0 {
            sleep(Duration::from_millis(delay_ms)).await;
        }
        Ok(())
    }

    pub async fn read_line_timeout(&mut self, timeout_ms: u64) -> Result<Option<String>> {
        let mut line = String::new();

        if timeout_ms == 0 {
            let n = self.reader.read_line(&mut line).await?;
            if n > 0 {
                // println!("<< {line}");
                return Ok(Some(line));
            } else {
                return Ok(None);
            }
        }

        match tokio::time::timeout(
            Duration::from_millis(timeout_ms),
            self.reader.read_line(&mut line),
        )
        .await
        {
            Ok(Ok(n)) if n > 0 => Ok(Some(line)),
            _ => Ok(None),
        }
    }

    pub async fn shutdown(&self) -> Result<()> {
        let mut writer = self.writer.lock().await;
        writer.shutdown().await?;
        Ok(())
    }

    pub async fn send_raw(&self, data: &[u8]) -> Result<()> {
        let mut writer = self.writer.lock().await;
        writer.write_all(data).await?;
        writer.flush().await?;
        Ok(())
    }
}
