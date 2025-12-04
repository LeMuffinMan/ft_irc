use anyhow::Result;
use std::sync::Arc;
use tokio::sync::Mutex;
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader, BufWriter},
    net::TcpStream,
    time::{Duration, sleep},
};

#[derive(Copy, Clone, Debug)]
pub enum ClientBehavior {
    LegitDisconnect,
    LegitIgnorePong,
    StartIgnoreAll,
    PongOnly,
    WrongPong,
    PongWithoutConnect,
    NormalConnection,
    WrongPassword,
    FragmentedMessages,
    LowBandwidth,
    ContinuousNoise,
    TooLongMessage,

    NickNormalClaimAndChange,
    NickNoNicknameGiven,
    // NickAlreadyInUse,
    PassAlreadyRregistered,
    PassNeedMoreParams,
    PassNotFirst,

    UserAlreadyRegistered,
    UserNeedMoreParams,

    InviteNeedMoreParams,
    // InviteNoSuchNick,
    // InviteNotOnChannel,

    PrivmsgNoRecipient,
    PrivmsgNoTextToSend,
    // PrivmsgNoSuchChannel,
    // PrivmsgCannotSendToChan,
    PrivmsgNoSuchNick,

    // KickBadChanMask,
    // KickNoSuchChannel,
    // KickNeedMoreParams,
    // KickChaNoPrivsNeeded,
    // KickUserNotInChannel,
    JoinNeedMoreParams,
    JoinNoSuchChan,

    TopicNeedMoreParams,
}

pub struct Client {
    writer: Arc<Mutex<BufWriter<tokio::net::tcp::OwnedWriteHalf>>>,
    reader: tokio::io::BufReader<tokio::net::tcp::OwnedReadHalf>,
}

impl Client {
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
