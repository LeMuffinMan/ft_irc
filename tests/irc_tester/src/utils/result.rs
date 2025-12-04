use crate::client::ClientBehavior;
use tokio::time::Duration;

#[derive(Debug)]
pub struct ClientResult {
    pub id: usize,
    pub behavior: ClientBehavior,
    pub success: bool,
    pub message: Option<String>,
    pub reply_time: Duration,
}

impl ClientResult {
    pub fn success(id: usize, behavior: ClientBehavior, reply_time: Duration) -> Self {
        Self {
            id,
            behavior,
            success: true,
            message: None,
            reply_time,
        }
    }

    pub fn failure(
        id: usize,
        behavior: ClientBehavior,
        message: String,
        reply_time: Duration,
    ) -> Self {
        Self {
            id,
            behavior,
            success: false,
            message: Some(message),
            reply_time,
        }
    }
}
