# Background channels and unread handling

Source audit: main loop services Mesh regardless of screen; Mesh tries matching configured channel hashes/keys and verifies MAC before BaseChatMesh dispatch. MyMesh calls onChannelMsg independently of BLE connection and current UI screen. UI stores incoming messages in the channel's own thread and queues its normal chime.

Fixes: notification popup no longer suppressed when Chat is open on a different thread or not showing latest content. Channel tabs show numeric unread counts. Read marking moves from enter/switch to actual latest-message drawing, after exclusive overlays return; background threads remain unread. Counts remain bounded to retained incoming messages as ring slots are overwritten. MF_UNREAD uses a previously unused flag bit in the existing persisted message record; markRead schedules a save, and load reconstructs counts from retained flags. Legacy history has no unread bits.

Validation command unreadtest uses an isolated RAM store, never loads/saves user history, checks independent counts across two channels, outgoing-message exclusion, markRead isolation, persistent flag state and ring overwrite. Real over-the-air channel reception is not simulated by this check.

Installed application hash verified. On-device unreadtest PASS. Home health check at 30 seconds: receiver active, RX errors zero, periodic recovery interval/count zero; GPS fixed, touch present. No incoming channel message observed during that initial window. No real chat history was changed by the test.
