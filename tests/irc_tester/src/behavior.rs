use crate::behaviors::nick::*;
use crate::behaviors::protocol::*;
use crate::behaviors::invite::*;
use crate::behaviors::join::*;
use crate::behaviors::kick::*;
use crate::behaviors::pass::*;
use crate::behaviors::ping_pong::*;
use crate::behaviors::privmsg::*;
use crate::behaviors::topic::*;
use crate::behaviors::user::*;

use anyhow::Result;
use std::future::Future;
use std::pin::Pin;

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
    InviteNoPriv,
    InviteNotRegistered,

    //privmsg
    PrivmsgNoRecipient,
    PrivmsgNoTextToSend,
    // PrivmsgNoSuchChannel,
    // PrivmsgCannotSendToChan,
    PrivmsgNoSuchNick,

    //kick
    // KickBadChanMask,
    KickNoSuchChannel, // -> segfault : isClientInChannel Channel.cpp 108 (invalid read of size)
    KickNeedMoreParams,
    // KickChaNoPrivsNeeded,
    // KickUserNotInChannel,

    //join
    JoinNeedMoreParams,
    JoinNoSuchChan,
    JoinNewChan,
    JoinNotRegistered,
    JoinInviteOnlyChannel,
    // JoinBadChannelKey,
    // JoinChannelIsFull,
    JoinExistingChan,
    JoinExistingChanMdp,
    JoinExistingMutliChan,

    //topic
    TopicNeedMoreParams,
    TopicNotOnChannel,
    TopicNoTopic,
    // TopicRpl,
    // TopicNoPriv,
    // TopicNoChanModes,
}

impl BehaviorHandler for ClientBehavior {
    fn handler(&self) -> Handler {
        use ClientBehavior::*;

        match self {
            // connection
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
            InviteNoPriv => |p, id, t| Box::pin(invite_no_priv(p, id, t)),
            InviteNotRegistered => |p, id, t| Box::pin(invite_not_registered(p, id, t)),

            PrivmsgNoRecipient => |p, id, t| Box::pin(privmsg_no_recipient(p, id, t)),
            PrivmsgNoTextToSend => |p, id, t| Box::pin(privmsg_no_text_to_send(p, id, t)),
            // PrivmsgNoSuchChannel => |p, id, t| Box::pin(privmsg_no_such_channel(p, id, t)),
            // PrivmsgCannotSendToChan => |p, id, t| Box::pin(privmsg_cannot_send_to_chan(p, id, t)),
            PrivmsgNoSuchNick => |p, id, t| Box::pin(privmsg_no_such_nick(p, id, t)),

            KickNeedMoreParams => |p, id, t| Box::pin(kick_need_more_params(p, id, t)),
            KickNoSuchChannel => |p, id, t| Box::pin(kick_no_such_channel(p, id, t)),

            JoinNeedMoreParams => |p, id, t| Box::pin(join_need_more_params(p, id, t)),
            JoinNoSuchChan => |p, id, t| Box::pin(join_no_such_channel(p, id, t)),
            JoinNewChan => |p, id, t| Box::pin(join_new_channel(p, id, t)),
            JoinNotRegistered => |p, id, t| Box::pin(join_not_registered(p, id, t)),
            JoinExistingMutliChan => |p, id, t| Box::pin(join_existing_multi_chan(p, id, t)),
            JoinInviteOnlyChannel => |p, id, t| Box::pin(join_invite_only_chan(p, id, t)),
            JoinExistingChanMdp => |p, id, t| Box::pin(join_existing_chan_mdp(p, id, t)),
            JoinExistingChan => |p, id, t| Box::pin(join_existing_chan(p, id, t)),
            // JoinBadChannelKey => |p, id, t| Box::pin(join_bad_channel_key(p, id, t)),
            // JoinChannelIsFull => |p, id, t| Box::pin(join_channel_is_full(p, id, t)),
            TopicNeedMoreParams => |p, id, t| Box::pin(topic_need_more_params(p, id, t)),
            TopicNotOnChannel => |p, id, t| Box::pin(topic_not_on_chan(p, id, t)),
            TopicNoTopic => |p, id, t| Box::pin(topic_no_topic(p, id, t)),
            // TopicRpl => |p, id, t| Box::pin(topic_reply(p, id, t)),
            // TopicNoPriv => |p, id, t| Box::pin(topic_no_priv(p, id, t)),
            // TopicNoChanModes => |p, id, t| Box::pin(topic_no_chan_modes(p, id, t)),
        }
    }
}
