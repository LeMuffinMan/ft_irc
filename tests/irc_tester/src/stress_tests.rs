use crate::behavior::BehaviorHandler;
use crate::behavior::ClientBehavior;
use crate::client::Client;
use crate::result::ClientResult;

use anyhow::Result;
use futures::stream::{FuturesUnordered, StreamExt};
use rand::Rng;
use rand::prelude::IndexedRandom;
use rand::rng;
use tokio::time::{Duration, Instant};


/**
 *
 * @brief Initialise and execute a test depending of Behavior
*   It expects a ClientResult struct including data collected during test.
 *
 */
pub async fn run_client(
    port: u16,
    id: usize,
    behavior: ClientBehavior,
    timeout_ms: u64,
) -> ClientResult {
    let handler = behavior.handler();
    let start = Instant::now();
    let result = handler(port, id, timeout_ms).await;
    let reply = Instant::now() - start;

    match result {
        Ok(_) => ClientResult::success(id, behavior, reply),
        Err(e) => ClientResult::failure(id, behavior, e.to_string(), reply),
    }
}

/**
 *
 * @brief Test each unit test in parrallele.
 *  
 */

pub async fn test_behaviors(port: u16, timeout_ms: u64) -> Result<()> {
    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::LegitIgnorePong,
        ClientBehavior::StartIgnoreAll,
        ClientBehavior::PongOnly,
        ClientBehavior::WrongPong,
        ClientBehavior::PongWithoutConnect,
        ClientBehavior::WrongPassword,
        ClientBehavior::FragmentedMessages,
        ClientBehavior::LowBandwidth,
        ClientBehavior::ContinuousNoise,
        ClientBehavior::TooLongMessage,
        ClientBehavior::NickNormalClaimAndChange,
        ClientBehavior::NickNoNicknameGiven,
        ClientBehavior::NickAlreadyInUse,
        ClientBehavior::PassAlreadyRregistered,
        ClientBehavior::PassNeedMoreParams,
        ClientBehavior::PassNotFirst,
        ClientBehavior::UserAlreadyRegistered,
        ClientBehavior::UserNeedMoreParams,
        ClientBehavior::InviteNeedMoreParams,
        ClientBehavior::InviteNoSuchNick,
        ClientBehavior::InviteNotOnChannel,
        ClientBehavior::InviteNotRegistered,
        ClientBehavior::PrivmsgNoRecipient,
        ClientBehavior::PrivmsgNoTextToSend,
        ClientBehavior::PrivmsgNoSuchChannel,
        ClientBehavior::PrivmsgToNick,
        ClientBehavior::PrivmsgToChan,
        ClientBehavior::PrivmsgNoSuchNick,
        ClientBehavior::PrivmsgNotRegistered,
        ClientBehavior::PrivmsgTooManyTargets,
        ClientBehavior::PrivmsgToNickNotSharingChan,
        ClientBehavior::KickNeedMoreParams,
        ClientBehavior::KickNotRegistered,
        ClientBehavior::KickNoSuchChannel,
        ClientBehavior::JoinNeedMoreParams,
        ClientBehavior::JoinNoSuchChan,
        ClientBehavior::JoinNewChan,
        ClientBehavior::JoinNotRegistered,
        ClientBehavior::JoinInviteOnlyChannel,
        ClientBehavior::JoinExistingMultiChan,
        ClientBehavior::JoinExistingChan,
        ClientBehavior::JoinExistingChanMdp,
        ClientBehavior::TopicNeedMoreParams,
        ClientBehavior::TopicNotRegistered,
        ClientBehavior::TopicNotOnChannel,
        ClientBehavior::TopicNoTopic,
        ClientBehavior::ModeNeedMoreParams,
        ClientBehavior::ModeNotRegistered,
        ClientBehavior::PartNeedMoreParams,
        ClientBehavior::PartNotRegistered,
        ClientBehavior::TimeWithParams,
        ClientBehavior::TimeCheckTimeZone,
        ClientBehavior::TimeNotRegistered,
        ClientBehavior::InviteModeIJoin,
        ClientBehavior::KickPriv,
    ];

    // We use FuturesUnordered and we do not await our async task, to send them by interating in
    // behavior enum
    // for each behavior, we create an asynchron future, which realise the test.
    // .collect() fill our futures variable, with our asynchron function return 
    let mut futures: FuturesUnordered<_> = behaviors
        .into_iter()
        .map(|behavior| async move {
            let result = run_client(port, 0, behavior, timeout_ms).await;
            (behavior, result)
        })
        .collect();

    let mut res = Ok(());

    //we can now await each result and access tests data
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
/**
 *
 * @brief We run 100 - <num_clients> clients each 100 - 300 ms, using a random behavior
 *  
 */
pub async fn advanced_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
    println!("Starting advanced stress test with waves of clients for 10 seconds...");

    let behaviors = vec![
        ClientBehavior::LegitDisconnect,
        ClientBehavior::FragmentedMessages,
        ClientBehavior::LowBandwidth,
        ClientBehavior::TooLongMessage,
        ClientBehavior::NickNormalClaimAndChange,
        ClientBehavior::NickNoNicknameGiven,
        ClientBehavior::WrongPassword,
        ClientBehavior::PassAlreadyRregistered,
        ClientBehavior::PassNotFirst,
        ClientBehavior::PassNeedMoreParams,
        ClientBehavior::UserAlreadyRegistered,
        ClientBehavior::UserNeedMoreParams,
        ClientBehavior::InviteNeedMoreParams,
        ClientBehavior::InviteNotOnChannel,
        ClientBehavior::NickAlreadyInUse,
        ClientBehavior::InviteNotRegistered,
        ClientBehavior::PrivmsgNoRecipient,
        ClientBehavior::PrivmsgNoTextToSend,
        ClientBehavior::PrivmsgNoSuchChannel,
        ClientBehavior::PrivmsgTooManyTargets,
        ClientBehavior::KickNeedMoreParams,
        ClientBehavior::KickNotRegistered,
        ClientBehavior::KickNoSuchChannel,
        ClientBehavior::KickPriv,
        ClientBehavior::JoinInviteOnlyChannel,
        ClientBehavior::ModeNeedMoreParams,
        ClientBehavior::ModeNotRegistered,
        ClientBehavior::PartNeedMoreParams,
        ClientBehavior::PartNotRegistered,
        ClientBehavior::TimeWithParams,
        ClientBehavior::TimeCheckTimeZone,
        ClientBehavior::TimeNotRegistered,
    ];

    let mut handles = vec![];
    let start_time = Instant::now();
    let duration = Duration::from_secs(10);
    let mut client_id = 0;
    let mut rng = rng();

    //we use rng to diversify waves size, behaviors and frequency
    //we collect the future reutrned by spawn and we do not await our spawns here, to send waves without waiting the previous
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

    //now we collect and process test data
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

/**
 *
 * @brief We run  <num_clients> clients at almost same time, realising a normal connection test
 *  
 */

pub async fn connection_stress_test(port: u16, num_clients: usize, timeout_ms: u64) -> Result<()> {
    println!(
        "Starting connection stress test with {} clients...",
        num_clients
    );

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
                format!("NICK {}_stress_connection\r\n", nick),
                format!("USER {} 0 * :stress user\r\n", nick),
            ];

            for msg in messages {
                client.send(&msg, 0).await?;
            }

            if let Some(line) = client.read_line_timeout(timeout_ms).await? {
                if line.contains("Welcome to the Internet Relay Network") {
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

    let total_duration = start.elapsed();
    println!(
        "\nConnection stress test finished: {} \x1b[32mOK\x1b[0m, {} \x1b[31mKO\x1b[0m",
        ok_count, ko_count
    );
    println!("Total time: {:?}", total_duration);
    println!(
        "Time per client: {:?}\n",
        total_duration / num_clients as u32
    );
    Ok(())
}
