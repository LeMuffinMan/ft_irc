use crate::behavior::ClientBehavior;
use tokio::time::Duration;

/**
*
* @Brief ClientResult as test data capsule
* Each client is identified to the server with an id in their registered nickname
* Each has a behavior, the test.
* It include an optionnal message of error, including the last received line
* It include the test duration.
*
*/
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
