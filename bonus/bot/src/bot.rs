use anyhow::Result;
use std::sync::Arc;
use tokio::sync::Mutex;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader, BufWriter},
    net::TcpStream,
    time::{Duration, sleep},
};

pub struct Bot {
    writer: Arc<Mutex<BufWriter<tokio::net::tcp::OwnedWriteHalf>>>,
    reader: tokio::io::BufReader<tokio::net::tcp::OwnedReadHalf>,
}

impl Bot {
    pub async fn try_expect(
        &mut self,
        cmd: &str,
        expect: &str,
        error: &str,
        timeout_ms: u64,
    ) -> Result<()> {
        self.send(cmd, 0).await?;
        if let Some(line) = self.read_line_timeout(timeout_ms).await? {
            if !line.contains(expect) {
                return Err(anyhow::anyhow!("{} | Received [{}]", error, line));
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

    pub async fn get_user_nick(&mut self, timeout_ms: u64) -> Option<String> {
        if let Ok(Some(line)) = self.read_line_timeout(timeout_ms).await {
            println!("Received line = {line}");
            if line.contains("JOIN") {
                if let Some((nick, _)) = line.split_once(' ') {
                    let trimed_nick = nick.chars().skip(1).collect::<String>();
                    println!("Extracted Nick = {trimed_nick}");
                    return Some(trimed_nick.to_string());
                } else {
                    println!("Bot failed to get nick");
                }
            }
        }
        None
    }

    pub async fn pose_riddle(&mut self, riddle: String, nick_player: &String, timeout_ms: u64) -> Result<bool, ()> {
        if let Ok(_) = self.send(&riddle, timeout_ms).await {
            std::thread::sleep(std::time::Duration::from_millis(200));
            if let Ok(Some(player_answer)) = self.read_line_timeout(timeout_ms).await {
                println!("Player_answer = {player_answer}");
                let trimed_answer = player_answer.rfind(':');
                println!("trimed_answer = {:?}", trimed_answer);
                if  player_answer.ends_with(":2\r\n") {
                    let _ = self.send(&format!("PRIVMSG {nick_player} :Huh. There isn't enough neurotoxin to kill you. So I guess you win.\nTake this Aperture Science Handheld Portal Device, it does not make portal anymore but it translates robot languages\r\n"), timeout_ms).await;
                    println!("Good answer");
                    return Ok(true);
                } else if player_answer.ends_with(":1\r\n") {
                    println!("Bad answer");
                    let _ = self.send(&format!("PRIVMSG {nick_player} :Uh oh. Somebody cut the cake. I told them to wait for you, but they did it anyway. There is still some left, though, if you hurry back.\r\n"), timeout_ms).await;
                } else {
                    println!("Bad answer");
                    let _ = self.send(&format!("PRIVMSG {nick_player} :You are just as smart as you seem.\r\n"), timeout_ms).await;
                }
                return Ok(false);
            } else {
                return Err(());
            }
        }
        Err(())
    }
}
