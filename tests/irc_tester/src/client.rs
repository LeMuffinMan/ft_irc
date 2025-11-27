use anyhow::Result;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
    time::{Duration, sleep},
};

pub struct Client {
    reader: BufReader<tokio::net::tcp::OwnedReadHalf>,
    writer: tokio::net::tcp::OwnedWriteHalf,
}

impl Client {
    pub async fn connect(port: u16) -> Result<Self> {
        let stream = TcpStream::connect(("127.0.0.1", port)).await?;
        let (reader, writer) = stream.into_split();
        Ok(Self {
            reader: BufReader::new(reader),
            writer,
        })
    }

    pub async fn send(&mut self, msg: &str, delay_ms: u64) -> Result<()> {
        self.writer.write_all(msg.as_bytes()).await?;
        self.writer.flush().await?;
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

    pub async fn shutdown(&mut self) -> Result<()> {
        self.writer.shutdown().await?;
        Ok(())
    }
}
