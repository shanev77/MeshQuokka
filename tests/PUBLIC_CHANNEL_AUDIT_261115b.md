# Public channel audit

User confirmed Spoonie's Hi johan appeared inside Public, establishing successful channel reception rather than only a private DM. Audit found no receiver-name/mention filtering in Mesh group decoding, BaseChatMesh text handling, MyMesh callback or UI storage. Flood filtering returns false. Existing routing/key/MAC/duplicate rules remain enforced.

Before update, read-only USB inspection without reset recorded RX=28, RX errors=3, TX=27; all 28 raw metadata entries still fit the history. Four were channel text packets, 20 adverts, and the remaining four request/response/control. Advert counters were valid=2/self=18. Device error flags=0, recovery interval/count=0. This does not show whether all four channel frames decoded, nor establish reception of missing internet-observed traffic.

Version b adds channel processing-stage counters and RAM-history success/failure reporting. It preserves channel acceptance behavior, except adding a bounds check before accessing the channel byte in a short frame. It exposes no message contents or keys in the counters. RAM-only channeltest uses the actual encryption, channel-hash lookup, MAC/decryption, text parser and callback, intercepting the callback before offline queues/UI/history. It does not simulate a radio reception or claim coverage.

Installed application hash verified. On-device channeltest PASS for general/named/general text, and unreadtest PASS. After tests all channel counters remained zero and TX=0, confirming no synthetic packets were sent or counted as live traffic. Hardware device errors=0, RX active, periodic recovery disabled. The tests establish local decoding/callback and storage-counter behavior, not successful reception of every internet-observed message.
