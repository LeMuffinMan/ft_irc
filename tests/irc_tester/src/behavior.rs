use crate::behaviors::invite::*;
use crate::behaviors::join::*;
use crate::behaviors::kick::*;
use crate::behaviors::mode::*;
use crate::behaviors::nick::*;
use crate::behaviors::part::*;
use crate::behaviors::pass::*;
use crate::behaviors::ping_pong::*;
use crate::behaviors::privmsg::*;
use crate::behaviors::protocol::*;
use crate::behaviors::time::*;
use crate::behaviors::topic::*;
use crate::behaviors::user::*;

use anyhow::Result;
use std::future::Future;
use std::pin::Pin;


/**
*
*
* @Brief Handler trait 
*
*  To decouple the list of client behaviors from the execution logic,
 * we useda a handler-based abstraction.
 *
 * Each ClientBehavior variant represents a specific test scenario
 *
 * A behavior is associated with a handler function through the
 * `BehaviorHandler` trait. The handler is a function pointer returning
 * an asynchronous task (`Future`) executing the corresponding scenario.
 *
 * This design allows:
 * - clean separation between behavior definition and execution
 * - easy addition of new behaviors without modifying the execution engine
 * - dynamic dispatch of asynchronous test scenarios
 *
 * The Handler type defines a uniform signature for all behavior handlers:
 * (port, client_id, timeout) -> Future<Result<()>>.
*
* Each test are detailed in behaviors folder
*
 */
pub type Handler = fn(u16, usize, u64) -> Pin<Box<dyn Future<Output = Result<()>> + Send>>;

pub trait BehaviorHandler {
    fn handler(&self) -> Handler;
}

#[derive(Copy, Clone, Debug)]
pub enum ClientBehavior {
    //connection
    LegitDisconnect,
    FragmentedMessages,
    LowBandwidth,
    ContinuousNoise,
    TooLongMessage,

    //pong
    LegitIgnorePong,
    StartIgnoreAll,
    PongOnly,
    WrongPong,
    PongWithoutConnect,

    //nick
    NickNormalClaimAndChange,
    NickNoNicknameGiven,
    NickAlreadyInUse,

    //pass
    WrongPassword,
    PassAlreadyRregistered,
    PassNeedMoreParams,
    PassNotFirst,

    //User
    UserAlreadyRegistered,
    UserNeedMoreParams,

    //Invite
    InviteNeedMoreParams,
    InviteNoSuchNick,
    InviteNotOnChannel,
    InviteNotRegistered,
    InviteModeIJoin,

    //privmsg
    PrivmsgNoRecipient,
    PrivmsgNoTextToSend,
    PrivmsgNoSuchChannel,
    PrivmsgNoSuchNick,
    PrivmsgToNick,
    PrivmsgToChan,
    PrivmsgTooManyTargets,
    PrivmsgToNickNotSharingChan,
    PrivmsgNotRegistered,

    //kick
    KickNoSuchChannel,
    KickNeedMoreParams,
    KickNotRegistered,
    KickPriv,

    //join
    JoinNeedMoreParams,
    JoinNoSuchChan,
    JoinNewChan,
    JoinNotRegistered,
    JoinInviteOnlyChannel,
    JoinExistingChan,
    JoinExistingChanMdp,
    JoinExistingMultiChan,

    //topic
    TopicNeedMoreParams,
    TopicNotRegistered,
    TopicNotOnChannel,
    TopicNoTopic,

    //Mode
    ModeNeedMoreParams,
    ModeNotRegistered,

    //Part
    PartNeedMoreParams,
    PartNotRegistered,

    //Time
    TimeWithParams,
    TimeCheckTimeZone,
    TimeNotRegistered,
}

impl BehaviorHandler for ClientBehavior {
    fn handler(&self) -> Handler {
        use ClientBehavior::*;

        match self {
            FragmentedMessages => |p, id, _| Box::pin(fragmented_messages(p, false, id)),
            LowBandwidth => |p, id, _| Box::pin(low_bandwidth(p, false, id)),
            LegitDisconnect => |p, id, t| Box::pin(legit_disconnect(p, id, t)),
            ContinuousNoise => |p, _id, t| Box::pin(continuous_noise(p, t)),
            TooLongMessage => |p, _id, t| Box::pin(too_long_message(p, t)),

            WrongPong => |p, id, t| Box::pin(wrong_pong(p, id, t)),
            PongWithoutConnect => |p, _id, t| Box::pin(pong_without_connect(p, t)),
            LegitIgnorePong => |p, id, t| Box::pin(legit_ignore_pong(p, id, t)),
            StartIgnoreAll => |p, id, t| Box::pin(start_ignore_all(p, id, t)),
            PongOnly => |p, id, t| Box::pin(pong_only(p, id, t)),

            NickNormalClaimAndChange => |p, id, t| Box::pin(nick_normal_claim_and_change(p, id, t)),
            NickNoNicknameGiven => |p, id, t| Box::pin(nick_no_nickname_given(p, id, t)),
            NickAlreadyInUse => |p, id, t| Box::pin(nick_already_in_use(p, id, t)),

            WrongPassword => |p, _id, _t| Box::pin(normal_connection_wrong_password(p, false)),
            PassNotFirst => |p, id, t| Box::pin(pass_not_first(p, id, t)),
            PassAlreadyRregistered => |p, id, t| Box::pin(pass_already_registered(p, id, t)),
            PassNeedMoreParams => |p, id, t| Box::pin(pass_need_more_params(p, id, t)),

            UserAlreadyRegistered => |p, id, t| Box::pin(user_already_registered(p, id, t)),
            UserNeedMoreParams => |p, id, t| Box::pin(user_need_more_params(p, id, t)),

            InviteNeedMoreParams => |p, id, t| Box::pin(invite_need_more_params(p, id, t)),
            InviteNoSuchNick => |p, id, t| Box::pin(invite_no_such_nick(p, id, t)), //
            InviteNotOnChannel => |p, id, t| Box::pin(invite_not_on_channel(p, id, t)),
            InviteNotRegistered => |p, id, t| Box::pin(invite_not_registered(p, id, t)),

            PrivmsgNoRecipient => |p, id, t| Box::pin(privmsg_no_recipient(p, id, t)),
            PrivmsgNoTextToSend => |p, id, t| Box::pin(privmsg_no_text_to_send(p, id, t)),
            PrivmsgNoSuchChannel => |p, id, t| Box::pin(privmsg_no_such_channel(p, id, t)),
            PrivmsgNoSuchNick => |p, id, t| Box::pin(privmsg_no_such_nick(p, id, t)),
            PrivmsgToNick => |p, id, t| Box::pin(privmsg_to_nick(p, id, t)),
            PrivmsgToChan => |p, id, t| Box::pin(privmsg_to_chan(p, id, t)),
            PrivmsgTooManyTargets => |p, id, t| Box::pin(privmsg_too_many_targets(p, id, t)),
            PrivmsgToNickNotSharingChan => {
                |p, id, t| Box::pin(privmsg_to_nick_not_sharing_chan(p, id, t))
            }
            PrivmsgNotRegistered => |p, id, t| Box::pin(privmsg_not_registered(p, id, t)),

            KickNeedMoreParams => |p, id, t| Box::pin(kick_need_more_params(p, id, t)),
            KickNoSuchChannel => |p, id, t| Box::pin(kick_no_such_channel(p, id, t)),
            KickNotRegistered => |p, id, t| Box::pin(kick_not_registered(p, id, t)),

            JoinNeedMoreParams => |p, id, t| Box::pin(join_need_more_params(p, id, t)),
            JoinNoSuchChan => |p, id, t| Box::pin(join_no_such_channel(p, id, t)),
            JoinNewChan => |p, id, t| Box::pin(join_new_channel(p, id, t)),
            JoinNotRegistered => |p, id, t| Box::pin(join_not_registered(p, id, t)),
            JoinExistingMultiChan => |p, id, t| Box::pin(join_existing_multi_chan(p, id, t)),
            JoinInviteOnlyChannel => |p, id, t| Box::pin(join_invite_only_chan(p, id, t)),
            JoinExistingChanMdp => |p, id, t| Box::pin(join_existing_chan_mdp(p, id, t)),
            JoinExistingChan => |p, id, t| Box::pin(join_existing_chan(p, id, t)),

            TopicNeedMoreParams => |p, id, t| Box::pin(topic_need_more_params(p, id, t)),
            TopicNotRegistered => |p, id, t| Box::pin(topic_not_registered(p, id, t)),
            TopicNotOnChannel => |p, id, t| Box::pin(topic_not_on_chan(p, id, t)),
            TopicNoTopic => |p, id, t| Box::pin(topic_no_topic(p, id, t)),
            ModeNeedMoreParams => |p, id, t| Box::pin(mode_need_more_params(p, id, t)),
            ModeNotRegistered => |p, id, t| Box::pin(mode_not_registered(p, id, t)),

            PartNeedMoreParams => |p, id, t| Box::pin(part_need_more_params(p, id, t)),
            PartNotRegistered => |p, id, t| Box::pin(part_not_registered(p, id, t)),

            TimeWithParams => |p, id, t| Box::pin(time_with_params(p, id, t)),
            TimeCheckTimeZone => |p, id, t| Box::pin(time_check_answer(p, id, t)),
            TimeNotRegistered => |p, id, t| Box::pin(time_not_registered(p, id, t)),

            KickPriv => |p, id, t| Box::pin(kick_priviledges(p, id, t)),
            InviteModeIJoin => |p, id, t| Box::pin(invite_mode_i_join(p, id, t)),
        }
    }
}
