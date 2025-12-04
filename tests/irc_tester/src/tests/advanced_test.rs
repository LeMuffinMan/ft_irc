use anyhow::Result;
use futures::stream::{FuturesUnordered, StreamExt};
use rand::Rng;
use rand::prelude::IndexedRandom;
use rand::rng;
use tokio::time::{Duration, Instant};

use crate::behaviors::*;
use crate::client::ClientBehavior;
use crate::utils::ClientResult;

async fn run_client(
    port: u16,
    id: usize,
    behavior: ClientBehavior,
    timeout_ms: u64,
) -> ClientResult {
    let start_time = Instant::now();
    let result: anyhow::Result<()> = match behavior {
        ClientBehavior::NormalConnection => normal_connection(port, false, id).await,
        ClientBehavior::WrongPassword => normal_connection_wrong_password(port, false).await,
        ClientBehavior::FragmentedMessages => fragmented_messages(port, false, id).await,
        ClientBehavior::LowBandwidth => low_bandwidth(port, false, id).await,
        ClientBehavior::PassNotFirst => pass_not_first(port, id, timeout_ms).await,
        ClientBehavior::LegitDisconnect => legit_disconnect(port, id, timeout_ms).await,
        ClientBehavior::LegitIgnorePong => legit_ignore_pong(port, id, timeout_ms).await,
        ClientBehavior::StartIgnoreAll => start_ignore_all(port, id, timeout_ms).await,
        ClientBehavior::PongOnly => pong_only(port, id, timeout_ms).await,
        ClientBehavior::WrongPong => wrong_pong(port, id, timeout_ms).await,
        ClientBehavior::PongWithoutConnect => pong_without_connect(port, timeout_ms).await,
        ClientBehavior::ContinuousNoise => continuous_noise(port, timeout_ms).await,
        ClientBehavior::TooLongMessage => too_long_message(port, timeout_ms).await,
        ClientBehavior::NickNormalClaimAndChange => {
            nick_normal_claim_and_change(port, id, timeout_ms).await
        }
        ClientBehavior::NickNoNicknameGiven => nick_no_nickname_given(port, id, timeout_ms).await,
        ClientBehavior::PassAlreadyRregistered => {
            pass_already_registered(port, id, timeout_ms).await
        }
        ClientBehavior::PassNeedMoreParams => pass_need_more_params(port, id, timeout_ms).await,
        ClientBehavior::UserAlreadyRegistered => {
            user_already_registered(port, id, timeout_ms).await
        }
        ClientBehavior::UserNeedMoreParams => user_need_more_params(port, id, timeout_ms).await,
        ClientBehavior::InviteNeedMoreParams => invite_need_more_params(port, id, timeout_ms).await,
        ClientBehavior::InviteNoSuchNick => invite_no_such_nick(port, id, timeout_ms).await,
        ClientBehavior::InviteNotOnChannel => invite_not_on_channel(port, id, timeout_ms).await,
        ClientBehavior::PrivmsgNoRecipient => privmsg_no_recipient(port, id, timeout_ms).await,
        ClientBehavior::PrivmsgNoTextToSend => privmsg_no_text_to_send(port, id, timeout_ms).await,
        ClientBehavior::PrivmsgNoSuchChannel => privmsg_no_such_channel(port, id, timeout_ms).await,
        // ClientBehavior::PrivmsgCannotSendToChan => privmsg_cannot_send_to_chan(port, id, timeout_ms).await,
        ClientBehavior::PrivmsgNoSuchNick => privmsg_no_such_nick(port, id, timeout_ms).await,
        ClientBehavior::KickNeedMoreParams => kick_need_more_params(port, id, timeout_ms).await,
        ClientBehavior::KickNoSuchChannel => kick_no_such_channel(port, id, timeout_ms).await,
        ClientBehavior::JoinNeedMoreParams => join_need_more_params(port, id, timeout_ms).await,
        ClientBehavior::JoinNoSuchChan => join_no_such_channel(port, id, timeout_ms).await,
        ClientBehavior::TopicNeedMoreParams => topic_need_more_params(port, id, timeout_ms).await,
    };

    let reply_time = Instant::now() - start_time;
    // println!("reply time = {}ms", reply_time.as_millis());
    match result {
        Ok(_) => ClientResult::success(id, behavior, reply_time),
        Err(e) => ClientResult::failure(id, behavior, format!("{}", e), reply_time),
    }
}

pub async fn test_behaviors(port: u16, timeout_ms: u64) -> Result<()> {
    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::LegitIgnorePong,
        ClientBehavior::StartIgnoreAll,
        ClientBehavior::PongOnly,
        ClientBehavior::WrongPong,
        ClientBehavior::PongWithoutConnect,
        ClientBehavior::NormalConnection,
        ClientBehavior::WrongPassword,
        ClientBehavior::FragmentedMessages,
        ClientBehavior::LowBandwidth,
        ClientBehavior::ContinuousNoise,
        ClientBehavior::TooLongMessage,
        ClientBehavior::NickNormalClaimAndChange,
        ClientBehavior::NickNoNicknameGiven,
        ClientBehavior::PassAlreadyRregistered,
        ClientBehavior::PassNeedMoreParams, // should be kicked
        ClientBehavior::PassNotFirst,       // should be kicked
        ClientBehavior::UserAlreadyRegistered,
        ClientBehavior::UserNeedMoreParams,
        ClientBehavior::InviteNeedMoreParams,
        ClientBehavior::InviteNoSuchNick,
        ClientBehavior::InviteNotOnChannel,
        ClientBehavior::PrivmsgNoRecipient,
        ClientBehavior::PrivmsgNoTextToSend,
        ClientBehavior::PrivmsgNoSuchChannel,
        // ClientBehavior::PrivmsgCannotSendToChan,
        ClientBehavior::PrivmsgNoSuchNick,
        ClientBehavior::KickNeedMoreParams,
        // ClientBehavior::KickBadChanMask,
        ClientBehavior::KickNoSuchChannel,
        // ClientBehavior::KickChaNoPrivsNeeded,
        // ClientBehavior::KickUserNotInChannel,
        ClientBehavior::JoinNeedMoreParams,
        ClientBehavior::JoinNoSuchChan,
        ClientBehavior::TopicNeedMoreParams,
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
                println!("{:?} \x1b[32mOK\x1b[0m", behavior);
            }
            ClientResult {
                success: false,
                message,
                ..
            } => {
                println!("{:?} \x1b[31mKO\x1b[0m failed: {:?}", behavior, message);
                res = Err(anyhow::anyhow!(format!(
                    "{:?} \x1b[31mKO\x1b[0m: {:?}",
                    behavior, message
                )));
            }
        }
    }

    res
}

pub async fn advanced_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
    println!("Starting advanced stress test with waves of clients for 10 seconds...");

    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::NormalConnection,
        // ClientBehavior::WrongPassword, //should be kicked
        // ClientBehavior::FragmentedMessages,
        // ClientBehavior::LowBandwidth,
        ClientBehavior::TooLongMessage,
        ClientBehavior::NickNormalClaimAndChange,
        ClientBehavior::NickNoNicknameGiven,
        ClientBehavior::PassAlreadyRregistered,
        // ClientBehavior::PassNotFirst, //should be kicked
        ClientBehavior::PassNeedMoreParams,
        ClientBehavior::UserAlreadyRegistered,
        ClientBehavior::UserNeedMoreParams,
        // ClientBehavior::InviteNeedMoreParams,
        // ClientBehavior::InviteNoSuchNick, //getClient fail
        // ClientBehavior::InviteNotOnChannel, //getClient fail ?
    ];

    let mut handles = vec![];
    let start_time = Instant::now();
    let duration = Duration::from_secs(10);
    let mut client_id = 0;
    let mut rng = rng();

    while start_time.elapsed() < duration {
        let wave_size = rng.random_range(100..=num_clients);

        println!("Launching wave of {} clients...", wave_size);

        for _ in 0..wave_size {
            let behavior = *behaviors.choose(&mut rng).unwrap();
            let handle = tokio::spawn(run_client(port, client_id, behavior, timeout_ms));
            handles.push(handle);
            client_id += 1;
        }

        let delay = rng.random_range(100..=300);
        tokio::time::sleep(Duration::from_millis(delay)).await;
    }

    println!("10 seconds elapsed, waiting for all clients to finish...");

    let mut ok_count = 0;
    let mut ko_count = 0;
    let mut reply_time_vec: Vec<Duration> = Vec::new();

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
                reply_time_vec.push(client_result.reply_time);
            }
            Err(e) => {
                ko_count += 1;
                eprintln!("Client task panicked: {:?}", e);
            }
        }
    }

    let total: Duration = reply_time_vec.iter().copied().sum();
    let average = if !reply_time_vec.is_empty() {
        total / (reply_time_vec.len() as u32)
    } else {
        Duration::ZERO
    };

    println!(
        "\nAdvanced Stress test finished:\n\
     Total clients: {}\n\
     Average reply delay: {} ms\n\
     \x1b[32mOK\x1b[0m: {}\n\
     \x1b[31mKO\x1b[0m: {}\n",
        client_id,
        average.as_millis(),
        ok_count,
        ko_count
    );

    Ok(())
}
