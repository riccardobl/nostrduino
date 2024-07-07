# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`namespace `[`nostr`](#namespacenostr) | 
`namespace `[`nostr::esp32`](#namespacenostr_1_1esp32) | 
`namespace `[`nostr::esp32::ESP32Platform`](#namespacenostr_1_1esp32_1_1ESP32Platform) | 
`struct `[`AES_ctx`](#structAES__ctx) | 

# namespace `nostr` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`nostr::Nip04`](#classnostr_1_1Nip04) | 
`class `[`nostr::Nip47Response`](#classnostr_1_1Nip47Response) | 
`class `[`nostr::Nip47`](#classnostr_1_1Nip47) | 
`class `[`nostr::NostrEventTags`](#classnostr_1_1NostrEventTags) | 
`class `[`nostr::NostrEvent`](#classnostr_1_1NostrEvent) | Base class for [NostrEvent](#classnostr_1_1NostrEvent)
`class `[`nostr::UnsignedNostrEvent`](#classnostr_1_1UnsignedNostrEvent) | An unsigned [NostrEvent](#classnostr_1_1NostrEvent). This is a support class used to create a new nostr event. When the event is ready to be sent, it should be signed with the sign method, the result is a [SignedNostrEvent](#classnostr_1_1SignedNostrEvent) that can be sent to the relay.
`class `[`nostr::SignedNostrEvent`](#classnostr_1_1SignedNostrEvent) | A signed [NostrEvent](#classnostr_1_1NostrEvent) as it is received by the client and sent to the relay.
`class `[`nostr::NostrSubscription`](#classnostr_1_1NostrSubscription) | 
`class `[`nostr::NostrRelay`](#classnostr_1_1NostrRelay) | 
`class `[`nostr::NostrPool`](#classnostr_1_1NostrPool) | The main class for interacting with the Nostr network. Handles subscriptions, relays, and event publishing.
`class `[`nostr::NWCResponseCallbackBase`](#classnostr_1_1NWCResponseCallbackBase) | 
`class `[`nostr::NWCResponseCallback`](#classnostr_1_1NWCResponseCallback) | 
`class `[`nostr::NWC`](#classnostr_1_1NWC) | A class to interact with the Nostr Wallet Connect services without dealing with the underlying transport
`class `[`nostr::Connection`](#classnostr_1_1Connection) | 
`class `[`nostr::Transport`](#classnostr_1_1Transport) | 
`class `[`nostr::Utils`](#classnostr_1_1Utils) | 
`struct `[`nostr::s_Invoice`](#structnostr_1_1s__Invoice) | 
`struct `[`nostr::s_TLVRecords`](#structnostr_1_1s__TLVRecords) | 
`struct `[`nostr::s_KeySend`](#structnostr_1_1s__KeySend) | 
`struct `[`nostr::s_PayInvoiceResponse`](#structnostr_1_1s__PayInvoiceResponse) | 
`struct `[`nostr::s_MultiPayInvoiceResponse`](#structnostr_1_1s__MultiPayInvoiceResponse) | 
`struct `[`nostr::s_PayKeySendResponse`](#structnostr_1_1s__PayKeySendResponse) | 
`struct `[`nostr::s_MultiPayKeySendResponse`](#structnostr_1_1s__MultiPayKeySendResponse) | 
`struct `[`nostr::s_MakeInvoiceResponse`](#structnostr_1_1s__MakeInvoiceResponse) | 
`struct `[`nostr::s_LookUpInvoiceResponse`](#structnostr_1_1s__LookUpInvoiceResponse) | 
`struct `[`nostr::s_Transaction`](#structnostr_1_1s__Transaction) | 
`struct `[`nostr::s_ListTransactionsResponse`](#structnostr_1_1s__ListTransactionsResponse) | 
`struct `[`nostr::s_GetBalanceResponse`](#structnostr_1_1s__GetBalanceResponse) | 
`struct `[`nostr::s_GetInfoResponse`](#structnostr_1_1s__GetInfoResponse) | 
`struct `[`nostr::s_NWCData`](#structnostr_1_1s__NWCData) | 
`struct `[`nostr::s_NostrEventTag`](#structnostr_1_1s__NostrEventTag) | 
`struct `[`nostr::s_EventStatusCallbackEntry`](#structnostr_1_1s__EventStatusCallbackEntry) | 

# class `nostr::Nip04` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`decrypt`](#classnostr_1_1Nip04_1a61cff1c4cb447c825520003e8ef9448f)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & privateKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & senderPubKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content)` | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`encrypt`](#classnostr_1_1Nip04_1a5bc588240576cf44d6a0664b4d205595)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & privateKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & recipientPubKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content)` | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`decrypt`](#classnostr_1_1Nip04_1a61cff1c4cb447c825520003e8ef9448f)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & privateKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & senderPubKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content)` 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`encrypt`](#classnostr_1_1Nip04_1a5bc588240576cf44d6a0664b4d205595)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & privateKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & recipientPubKeyHex,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content)` 

# class `nostr::Nip47Response` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`errorCode`](#classnostr_1_1Nip47Response_1adc416a01e02ac121f5e8e655bc1d5b08) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`errorMessage`](#classnostr_1_1Nip47Response_1a04f327c91a1aade410f342b6efc7a451) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`resultType`](#classnostr_1_1Nip47Response_1aee914909ea62c8ddd6f18e93aa6adb30) | 
`public T `[`result`](#classnostr_1_1Nip47Response_1a4a1f71e17c1e175256f4927a8294bfe8) | 
`public inline  `[`Nip47Response`](#classnostr_1_1Nip47Response_1ab81bd783b170fa8287fa93ae6a5614e9)`()` | 
`public inline  `[`Nip47Response`](#classnostr_1_1Nip47Response_1a0f57334b55cc736954dcdea618857734)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` errorCode,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` errorMessage,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` resultType,T result)` | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`errorCode`](#classnostr_1_1Nip47Response_1adc416a01e02ac121f5e8e655bc1d5b08) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`errorMessage`](#classnostr_1_1Nip47Response_1a04f327c91a1aade410f342b6efc7a451) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`resultType`](#classnostr_1_1Nip47Response_1aee914909ea62c8ddd6f18e93aa6adb30) 

#### `public T `[`result`](#classnostr_1_1Nip47Response_1a4a1f71e17c1e175256f4927a8294bfe8) 

#### `public inline  `[`Nip47Response`](#classnostr_1_1Nip47Response_1ab81bd783b170fa8287fa93ae6a5614e9)`()` 

#### `public inline  `[`Nip47Response`](#classnostr_1_1Nip47Response_1a0f57334b55cc736954dcdea618857734)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` errorCode,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` errorMessage,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` resultType,T result)` 

# class `nostr::Nip47` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Nip47`](#classnostr_1_1Nip47_1a832db110f750bae323891fc3113dc483)`()` | 
`public inline  `[`Nip47`](#classnostr_1_1Nip47_1afa506b09ad4df68c72e088336d408429)`(`[`Nip04`](#classnostr_1_1Nip04)` nip04,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` servicePubKey,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` userPrivKey)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`payInvoice`](#classnostr_1_1Nip47_1a4126792accc0be87b3b209918d1f903d)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice,unsigned long amount)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`multiPayInvoice`](#classnostr_1_1Nip47_1a6aa7e03b79ab36063c9dbbb95dd5a07c)`(std::initializer_list< `[`Invoice`](#namespacenostr_1ac0f491ca12edeaa060bf89f8349d1c94)` > invoices)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`payKeySend`](#classnostr_1_1Nip47_1af6c9c30bfcf1f9cab0fbac77e67eb9c9)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` pubkey,unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` preimage,std::initializer_list< `[`TLVRecords`](#namespacenostr_1ad0f4687c761ee6988b42826d3ea717f6)` > tlv)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`multiPayKeySend`](#classnostr_1_1Nip47_1a3b82c08c5d19006e22d7f027829b6002)`(std::initializer_list< `[`KeySend`](#namespacenostr_1a2f0adf8fb06541c0734900f1f37c6a77)` > keySends)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`makeInvoice`](#classnostr_1_1Nip47_1aa567dc8ea8b24bbac721724f79c156ec)`(unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` description,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` descriptionHash,unsigned long expiry)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`lookUpPaymentHash`](#classnostr_1_1Nip47_1a500ce5a6edd53194e6ddc035d0f4c50d)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` paymentHash)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`lookUpInvoice`](#classnostr_1_1Nip47_1a74760efce0406b4ca35811112c4cf1dd)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`listTransactions`](#classnostr_1_1Nip47_1aa4f00696a8b58ae43ea7582d915147d4)`(unsigned long from,unsigned long until,int limit,int offset,bool unpaid,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` type)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`getBalance`](#classnostr_1_1Nip47_1a086e38d7ee42d8d98f9e439b515f9cbb)`()` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`getInfo`](#classnostr_1_1Nip47_1ad3598c721308e98893c1af2526b6ca98)`()` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1a227c93d1f00eef2dc26964d5960fa870)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [PayInvoiceResponse`](#namespacenostr_1aabe8d230016fd5f70285adbcbe77fe23)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1ac4de81240475645085b1b4da58ecb727)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [MultiPayInvoiceResponse`](#namespacenostr_1a0fb78e894360c3b4167124456e18cc1b)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1a8d1c779b4af12e4bb8963dc3ebdeecab)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [PayKeySendResponse`](#namespacenostr_1a70810c5e5dbfe96e16ae12fd2990e6d6)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1a6585a69c98e19c9969d8bfa012b62fda)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [MultiPayKeySendResponse`](#namespacenostr_1af6e0f43f7f982a4b4fe1a1c097cf9de3)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1ad0c8190ae14427ec5b1522b661e07f75)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [MakeInvoiceResponse`](#namespacenostr_1a784f3d1260b727625d00d370c0ac9e9f)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1a1787418bb1c82a2af95e0b0a5cecb215)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [LookUpInvoiceResponse`](#namespacenostr_1a9357541c46480c78b10dbd47531ce2d7)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1ab17e76dcb6ebb9ff02385db61dcf2177)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [ListTransactionsResponse`](#namespacenostr_1a5fd7239e7510091c73ba2602f4a6bc3d)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1a91fc31e78d9bc639c249547d7ae83524)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [GetBalanceResponse`](#namespacenostr_1a42570ba6e2e70160a34b1746331f5bff)` > & out)` | 
`public void `[`parseResponse`](#classnostr_1_1Nip47_1a592690a536b36a84b1d35b8d1cbc6bbc)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [GetInfoResponse`](#namespacenostr_1af3dc0dbf7fa69605d5ad971a37c5fe7f)` > & out)` | 

## Members

#### `public inline  `[`Nip47`](#classnostr_1_1Nip47_1a832db110f750bae323891fc3113dc483)`()` 

#### `public inline  `[`Nip47`](#classnostr_1_1Nip47_1afa506b09ad4df68c72e088336d408429)`(`[`Nip04`](#classnostr_1_1Nip04)` nip04,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` servicePubKey,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` userPrivKey)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`payInvoice`](#classnostr_1_1Nip47_1a4126792accc0be87b3b209918d1f903d)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice,unsigned long amount)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`multiPayInvoice`](#classnostr_1_1Nip47_1a6aa7e03b79ab36063c9dbbb95dd5a07c)`(std::initializer_list< `[`Invoice`](#namespacenostr_1ac0f491ca12edeaa060bf89f8349d1c94)` > invoices)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`payKeySend`](#classnostr_1_1Nip47_1af6c9c30bfcf1f9cab0fbac77e67eb9c9)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` pubkey,unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` preimage,std::initializer_list< `[`TLVRecords`](#namespacenostr_1ad0f4687c761ee6988b42826d3ea717f6)` > tlv)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`multiPayKeySend`](#classnostr_1_1Nip47_1a3b82c08c5d19006e22d7f027829b6002)`(std::initializer_list< `[`KeySend`](#namespacenostr_1a2f0adf8fb06541c0734900f1f37c6a77)` > keySends)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`makeInvoice`](#classnostr_1_1Nip47_1aa567dc8ea8b24bbac721724f79c156ec)`(unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` description,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` descriptionHash,unsigned long expiry)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`lookUpPaymentHash`](#classnostr_1_1Nip47_1a500ce5a6edd53194e6ddc035d0f4c50d)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` paymentHash)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`lookUpInvoice`](#classnostr_1_1Nip47_1a74760efce0406b4ca35811112c4cf1dd)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`listTransactions`](#classnostr_1_1Nip47_1aa4f00696a8b58ae43ea7582d915147d4)`(unsigned long from,unsigned long until,int limit,int offset,bool unpaid,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` type)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`getBalance`](#classnostr_1_1Nip47_1a086e38d7ee42d8d98f9e439b515f9cbb)`()` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`getInfo`](#classnostr_1_1Nip47_1ad3598c721308e98893c1af2526b6ca98)`()` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1a227c93d1f00eef2dc26964d5960fa870)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [PayInvoiceResponse`](#namespacenostr_1aabe8d230016fd5f70285adbcbe77fe23)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1ac4de81240475645085b1b4da58ecb727)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [MultiPayInvoiceResponse`](#namespacenostr_1a0fb78e894360c3b4167124456e18cc1b)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1a8d1c779b4af12e4bb8963dc3ebdeecab)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [PayKeySendResponse`](#namespacenostr_1a70810c5e5dbfe96e16ae12fd2990e6d6)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1a6585a69c98e19c9969d8bfa012b62fda)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [MultiPayKeySendResponse`](#namespacenostr_1af6e0f43f7f982a4b4fe1a1c097cf9de3)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1ad0c8190ae14427ec5b1522b661e07f75)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [MakeInvoiceResponse`](#namespacenostr_1a784f3d1260b727625d00d370c0ac9e9f)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1a1787418bb1c82a2af95e0b0a5cecb215)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [LookUpInvoiceResponse`](#namespacenostr_1a9357541c46480c78b10dbd47531ce2d7)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1ab17e76dcb6ebb9ff02385db61dcf2177)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [ListTransactionsResponse`](#namespacenostr_1a5fd7239e7510091c73ba2602f4a6bc3d)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1a91fc31e78d9bc639c249547d7ae83524)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [GetBalanceResponse`](#namespacenostr_1a42570ba6e2e70160a34b1746331f5bff)` > & out)` 

#### `public void `[`parseResponse`](#classnostr_1_1Nip47_1a592690a536b36a84b1d35b8d1cbc6bbc)`(`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * response,`[`Nip47Response](#classnostr_1_1Nip47Response)< [GetInfoResponse`](#namespacenostr_1af3dc0dbf7fa69605d5ad971a37c5fe7f)` > & out)` 

# class `nostr::NostrEventTags` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`NostrEventTags`](#classnostr_1_1NostrEventTags_1aa3da6ebe9f51b305ea6abecc00b32c5d)`()` | 
`public inline unsigned int `[`count`](#classnostr_1_1NostrEventTags_1af893816d95d8514db7c99e34110d05fa)`()` | Get the number of tags
`public const std::vector< `[`NostrEventTag`](#namespacenostr_1ab1b5a1ecddfc940a74946e6dd11d8c53)` > & `[`getTags`](#classnostr_1_1NostrEventTags_1a0adcf1d628966f26ffaffa86fd90d64c)`() const` | Get all tags
`public const std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > & `[`getTag`](#classnostr_1_1NostrEventTags_1a1033d773b10d3ba70a06cd8f6582f4bc)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key) const` | Get the first occurrence of a tag with the given key
`public unsigned int `[`addTag`](#classnostr_1_1NostrEventTags_1ac82580cf67dc2e6b94e6b178a4730ce5)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key,std::initializer_list< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > values)` | Add a tag with a given key and set of values NB. You can add multiple tags with the same key
`public unsigned int `[`addTag`](#classnostr_1_1NostrEventTags_1ade17a83eeabc9c5e73ddd04b788f1891)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key,std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > values)` | Add a tag with a given key and set of values NB. You can add multiple tags with the same key
`public void `[`removeTag`](#classnostr_1_1NostrEventTags_1aa0ddc280674a884148aeb6c6811232de)`(unsigned int index)` | Remove a tag at a given index
`public void `[`removeTags`](#classnostr_1_1NostrEventTags_1afe03f28c1c86bcc1a9c45d903435e5d6)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key)` | Remove all occurrences of a tag with the given key
`public void `[`clearTags`](#classnostr_1_1NostrEventTags_1ab31ef86e2a9bc93b1a795394c5ce2615)`()` | Remove all tags
`public void `[`toJson`](#classnostr_1_1NostrEventTags_1a8b9c8af2dfc40c4523a0aabff4a533c5)`(JsonArray doc) const` | Output to json
`protected std::vector< `[`NostrEventTag`](#namespacenostr_1ab1b5a1ecddfc940a74946e6dd11d8c53)` > `[`tags`](#classnostr_1_1NostrEventTags_1a5732417af566a9d62a268aa414153662) | 

## Members

#### `public inline  `[`NostrEventTags`](#classnostr_1_1NostrEventTags_1aa3da6ebe9f51b305ea6abecc00b32c5d)`()` 

#### `public inline unsigned int `[`count`](#classnostr_1_1NostrEventTags_1af893816d95d8514db7c99e34110d05fa)`()` 

Get the number of tags

#### `public const std::vector< `[`NostrEventTag`](#namespacenostr_1ab1b5a1ecddfc940a74946e6dd11d8c53)` > & `[`getTags`](#classnostr_1_1NostrEventTags_1a0adcf1d628966f26ffaffa86fd90d64c)`() const` 

Get all tags

#### `public const std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > & `[`getTag`](#classnostr_1_1NostrEventTags_1a1033d773b10d3ba70a06cd8f6582f4bc)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key) const` 

Get the first occurrence of a tag with the given key

#### `public unsigned int `[`addTag`](#classnostr_1_1NostrEventTags_1ac82580cf67dc2e6b94e6b178a4730ce5)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key,std::initializer_list< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > values)` 

Add a tag with a given key and set of values NB. You can add multiple tags with the same key

#### `public unsigned int `[`addTag`](#classnostr_1_1NostrEventTags_1ade17a83eeabc9c5e73ddd04b788f1891)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key,std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > values)` 

Add a tag with a given key and set of values NB. You can add multiple tags with the same key

#### `public void `[`removeTag`](#classnostr_1_1NostrEventTags_1aa0ddc280674a884148aeb6c6811232de)`(unsigned int index)` 

Remove a tag at a given index

#### `public void `[`removeTags`](#classnostr_1_1NostrEventTags_1afe03f28c1c86bcc1a9c45d903435e5d6)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` key)` 

Remove all occurrences of a tag with the given key

#### `public void `[`clearTags`](#classnostr_1_1NostrEventTags_1ab31ef86e2a9bc93b1a795394c5ce2615)`()` 

Remove all tags

#### `public void `[`toJson`](#classnostr_1_1NostrEventTags_1a8b9c8af2dfc40c4523a0aabff4a533c5)`(JsonArray doc) const` 

Output to json

#### `protected std::vector< `[`NostrEventTag`](#namespacenostr_1ab1b5a1ecddfc940a74946e6dd11d8c53)` > `[`tags`](#classnostr_1_1NostrEventTags_1a5732417af566a9d62a268aa414153662) 

# class `nostr::NostrEvent` 

Base class for [NostrEvent](#classnostr_1_1NostrEvent)

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`NostrEvent`](#classnostr_1_1NostrEvent_1a4e0bb2a7e46e020b7da7dfcaca737719)`()` | 
`public inline virtual  `[`~NostrEvent`](#classnostr_1_1NostrEvent_1a0e5d5662e539f439e9071459a4d9dad5)`()` | 
`public inline unsigned long int `[`getCreatedAt`](#classnostr_1_1NostrEvent_1ae4371b0dadcb8fe08e8504a1437ff6b4)`()` | 
`public inline unsigned int `[`getKind`](#classnostr_1_1NostrEvent_1a7d66bdcdaad510b5c79d045b040ef99e)`()` | 
`public inline `[`NostrEventTags`](#classnostr_1_1NostrEventTags)` * `[`getTags`](#classnostr_1_1NostrEvent_1a752465b8cdfb66f6b730250af1a40fa5)`()` | 
`public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getContent`](#classnostr_1_1NostrEvent_1aebb7ff8a3985bdcf93548cd0520c5593)`()` | 
`protected unsigned int `[`kind`](#classnostr_1_1NostrEvent_1a669166a35df88ee656e39d5360414691) | 
`protected `[`NostrEventTags`](#classnostr_1_1NostrEventTags)` `[`tags`](#classnostr_1_1NostrEvent_1af3336c27b5d4158c9f20f42de5606bd5) | 
`protected unsigned long int `[`created_at`](#classnostr_1_1NostrEvent_1a1f0316b807c981423321b1ce288c8baf) | 
`protected `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`content`](#classnostr_1_1NostrEvent_1ad625c784ea6908e9f5f1d251718a2d08) | 
`protected inline  `[`NostrEvent`](#classnostr_1_1NostrEvent_1a5f917b73c6b8a19fe006aa906c25e50b)`(unsigned int kind,`[`NostrEventTags`](#classnostr_1_1NostrEventTags)` tags,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content,unsigned long int created_at)` | 

## Members

#### `public inline  `[`NostrEvent`](#classnostr_1_1NostrEvent_1a4e0bb2a7e46e020b7da7dfcaca737719)`()` 

#### `public inline virtual  `[`~NostrEvent`](#classnostr_1_1NostrEvent_1a0e5d5662e539f439e9071459a4d9dad5)`()` 

#### `public inline unsigned long int `[`getCreatedAt`](#classnostr_1_1NostrEvent_1ae4371b0dadcb8fe08e8504a1437ff6b4)`()` 

#### `public inline unsigned int `[`getKind`](#classnostr_1_1NostrEvent_1a7d66bdcdaad510b5c79d045b040ef99e)`()` 

#### `public inline `[`NostrEventTags`](#classnostr_1_1NostrEventTags)` * `[`getTags`](#classnostr_1_1NostrEvent_1a752465b8cdfb66f6b730250af1a40fa5)`()` 

#### `public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getContent`](#classnostr_1_1NostrEvent_1aebb7ff8a3985bdcf93548cd0520c5593)`()` 

#### `protected unsigned int `[`kind`](#classnostr_1_1NostrEvent_1a669166a35df88ee656e39d5360414691) 

#### `protected `[`NostrEventTags`](#classnostr_1_1NostrEventTags)` `[`tags`](#classnostr_1_1NostrEvent_1af3336c27b5d4158c9f20f42de5606bd5) 

#### `protected unsigned long int `[`created_at`](#classnostr_1_1NostrEvent_1a1f0316b807c981423321b1ce288c8baf) 

#### `protected `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`content`](#classnostr_1_1NostrEvent_1ad625c784ea6908e9f5f1d251718a2d08) 

#### `protected inline  `[`NostrEvent`](#classnostr_1_1NostrEvent_1a5f917b73c6b8a19fe006aa906c25e50b)`(unsigned int kind,`[`NostrEventTags`](#classnostr_1_1NostrEventTags)` tags,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content,unsigned long int created_at)` 

# class `nostr::UnsignedNostrEvent` 

```
class nostr::UnsignedNostrEvent
  : public nostr::NostrEvent
```  

An unsigned [NostrEvent](#classnostr_1_1NostrEvent). This is a support class used to create a new nostr event. When the event is ready to be sent, it should be signed with the sign method, the result is a [SignedNostrEvent](#classnostr_1_1SignedNostrEvent) that can be sent to the relay.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`UnsignedNostrEvent`](#classnostr_1_1UnsignedNostrEvent_1afddaa21707d84bf6b1a9e34576f5e7c9)`()` | 
`public inline  `[`UnsignedNostrEvent`](#classnostr_1_1UnsignedNostrEvent_1ace30203c0db0e13db4f949a6bf86ec97)`(unsigned int kind,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content,unsigned long int created_at)` | 
`public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`sign`](#classnostr_1_1UnsignedNostrEvent_1a0bcc2093ad6597c1910d09e1c7f9ed4c)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` privateKeyHex)` | Sign the event with a private key
`public inline void `[`setCreatedAt`](#classnostr_1_1UnsignedNostrEvent_1a7cc23ff66159de87a2407f89052b148a)`(unsigned long int created_at)` | 
`public inline void `[`setKind`](#classnostr_1_1UnsignedNostrEvent_1ab590c72570494a81b1c97f34b0f9bcd7)`(unsigned int kind)` | 
`public inline void `[`setContent`](#classnostr_1_1UnsignedNostrEvent_1a297f28145b27958863a5f15e27d1f64c)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content)` | 

## Members

#### `public inline  `[`UnsignedNostrEvent`](#classnostr_1_1UnsignedNostrEvent_1afddaa21707d84bf6b1a9e34576f5e7c9)`()` 

#### `public inline  `[`UnsignedNostrEvent`](#classnostr_1_1UnsignedNostrEvent_1ace30203c0db0e13db4f949a6bf86ec97)`(unsigned int kind,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content,unsigned long int created_at)` 

#### `public `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` `[`sign`](#classnostr_1_1UnsignedNostrEvent_1a0bcc2093ad6597c1910d09e1c7f9ed4c)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` privateKeyHex)` 

Sign the event with a private key

#### `public inline void `[`setCreatedAt`](#classnostr_1_1UnsignedNostrEvent_1a7cc23ff66159de87a2407f89052b148a)`(unsigned long int created_at)` 

#### `public inline void `[`setKind`](#classnostr_1_1UnsignedNostrEvent_1ab590c72570494a81b1c97f34b0f9bcd7)`(unsigned int kind)` 

#### `public inline void `[`setContent`](#classnostr_1_1UnsignedNostrEvent_1a297f28145b27958863a5f15e27d1f64c)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content)` 

# class `nostr::SignedNostrEvent` 

```
class nostr::SignedNostrEvent
  : public nostr::NostrEvent
```  

A signed [NostrEvent](#classnostr_1_1NostrEvent) as it is received by the client and sent to the relay.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getId`](#classnostr_1_1SignedNostrEvent_1a960a678a356d36a951f8758a3609725f)`()` | 
`public inline bool `[`isStored`](#classnostr_1_1SignedNostrEvent_1a13f3b9de76e6eba75ac93b8183ad0632)`()` | 
`public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getPubkey`](#classnostr_1_1SignedNostrEvent_1a3538024dd02de6db6c4348c7011eea4e)`()` | 
`public inline const `[`NostrEventTags`](#classnostr_1_1NostrEventTags)` * `[`getTags`](#classnostr_1_1SignedNostrEvent_1a8e8ceb84938452b7c22a801a8365410f)`() const` | 
`public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getSignature`](#classnostr_1_1SignedNostrEvent_1a172618d2515facba9a4814c19b17f186)`() const` | 
`public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getSubId`](#classnostr_1_1SignedNostrEvent_1aa8515e93f44bb3bf63add56aeddcb95c)`() const` | 
`public void `[`toSendableEvent`](#classnostr_1_1SignedNostrEvent_1a651d3b4a88cf616c81147feec9a5cfb1)`(JsonArray doc) const` | 
`public void `[`toJson`](#classnostr_1_1SignedNostrEvent_1a584d0a4b4a7971b0ae1564f085bd8b00)`(JsonObject doc) const` | 
`public bool `[`verify`](#classnostr_1_1SignedNostrEvent_1a3b4d5bdc0c1799b97cdff7198c0de2ad)`() const` | 
`protected  `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent_1aff5d7763948c60fbb3f422d77ae3207e)`(JsonArray relayEvent)` | 
`protected inline  `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent_1a18e71d3126700652a63cf97f26ad89cd)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` id,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` pubkey,unsigned long int created_at,unsigned int kind,`[`NostrEventTags`](#classnostr_1_1NostrEventTags)` tags,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` signature)` | 

## Members

#### `public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getId`](#classnostr_1_1SignedNostrEvent_1a960a678a356d36a951f8758a3609725f)`()` 

#### `public inline bool `[`isStored`](#classnostr_1_1SignedNostrEvent_1a13f3b9de76e6eba75ac93b8183ad0632)`()` 

#### `public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getPubkey`](#classnostr_1_1SignedNostrEvent_1a3538024dd02de6db6c4348c7011eea4e)`()` 

#### `public inline const `[`NostrEventTags`](#classnostr_1_1NostrEventTags)` * `[`getTags`](#classnostr_1_1SignedNostrEvent_1a8e8ceb84938452b7c22a801a8365410f)`() const` 

#### `public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getSignature`](#classnostr_1_1SignedNostrEvent_1a172618d2515facba9a4814c19b17f186)`() const` 

#### `public inline `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getSubId`](#classnostr_1_1SignedNostrEvent_1aa8515e93f44bb3bf63add56aeddcb95c)`() const` 

#### `public void `[`toSendableEvent`](#classnostr_1_1SignedNostrEvent_1a651d3b4a88cf616c81147feec9a5cfb1)`(JsonArray doc) const` 

#### `public void `[`toJson`](#classnostr_1_1SignedNostrEvent_1a584d0a4b4a7971b0ae1564f085bd8b00)`(JsonObject doc) const` 

#### `public bool `[`verify`](#classnostr_1_1SignedNostrEvent_1a3b4d5bdc0c1799b97cdff7198c0de2ad)`() const` 

#### `protected  `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent_1aff5d7763948c60fbb3f422d77ae3207e)`(JsonArray relayEvent)` 

#### `protected inline  `[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent_1a18e71d3126700652a63cf97f26ad89cd)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` id,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` pubkey,unsigned long int created_at,unsigned int kind,`[`NostrEventTags`](#classnostr_1_1NostrEventTags)` tags,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` content,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` signature)` 

# class `nostr::NostrSubscription` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool `[`eose`](#classnostr_1_1NostrSubscription_1a89dce3123587503b9995d65885be7c6e) | 
`public `[`NostrCloseCallback`](#namespacenostr_1a6247e694a00e0a5cb14ddf0929408fb5)` `[`closeCallback`](#classnostr_1_1NostrSubscription_1a41fa99234ae8c0448bb92f367948e688) | 
`public `[`NostrEOSECallback`](#namespacenostr_1a7a3b6e8f71428b2bf4195b93d35756ed)` `[`eoseCallback`](#classnostr_1_1NostrSubscription_1a787229e2f7460abb8919b587954f603d) | 
`public `[`NostrEventCallback`](#namespacenostr_1ab0da4b375d4d7eda0bb0bf1010f381bc)` `[`eventCallback`](#classnostr_1_1NostrSubscription_1abbb155e3799b9934e17757dac1e49675) | 
`public `[`NostrEventStatusCallback`](#namespacenostr_1a83396304773c199d3c6613e50113ac71)` `[`statusCallback`](#classnostr_1_1NostrSubscription_1a8347ef82aaa122cec6275381f53a2b55) | 

## Members

#### `public bool `[`eose`](#classnostr_1_1NostrSubscription_1a89dce3123587503b9995d65885be7c6e) 

#### `public `[`NostrCloseCallback`](#namespacenostr_1a6247e694a00e0a5cb14ddf0929408fb5)` `[`closeCallback`](#classnostr_1_1NostrSubscription_1a41fa99234ae8c0448bb92f367948e688) 

#### `public `[`NostrEOSECallback`](#namespacenostr_1a7a3b6e8f71428b2bf4195b93d35756ed)` `[`eoseCallback`](#classnostr_1_1NostrSubscription_1a787229e2f7460abb8919b587954f603d) 

#### `public `[`NostrEventCallback`](#namespacenostr_1ab0da4b375d4d7eda0bb0bf1010f381bc)` `[`eventCallback`](#classnostr_1_1NostrSubscription_1abbb155e3799b9934e17757dac1e49675) 

#### `public `[`NostrEventStatusCallback`](#namespacenostr_1a83396304773c199d3c6613e50113ac71)` `[`statusCallback`](#classnostr_1_1NostrSubscription_1a8347ef82aaa122cec6275381f53a2b55) 

# class `nostr::NostrRelay` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public void `[`send`](#classnostr_1_1NostrRelay_1aa52e9aac579b822a255080cbc12e6367)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` message)` | Send a message to the relay
`public inline  `[`NostrRelay`](#classnostr_1_1NostrRelay_1af668c3c57d9e84b398aac9ea481114e6)`(`[`Connection`](#classnostr_1_1Connection)` * conn,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` | 
`protected std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`messageQueue`](#classnostr_1_1NostrRelay_1a0c9e582eb5dc99a49ae4bf496052f89c) | 
`protected `[`Connection`](#classnostr_1_1Connection)` * `[`conn`](#classnostr_1_1NostrRelay_1ae456a4d62f0ede9c1dc6fc6e65f97679) | 
`protected `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`url`](#classnostr_1_1NostrRelay_1a36f2d542488906bf120ad44190c9c0d4) | 
`protected void `[`processQueue`](#classnostr_1_1NostrRelay_1aa03ceeb2e171b89412fbdab332205c0a)`()` | 

## Members

#### `public void `[`send`](#classnostr_1_1NostrRelay_1aa52e9aac579b822a255080cbc12e6367)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` message)` 

Send a message to the relay

#### `public inline  `[`NostrRelay`](#classnostr_1_1NostrRelay_1af668c3c57d9e84b398aac9ea481114e6)`(`[`Connection`](#classnostr_1_1Connection)` * conn,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` 

#### `protected std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`messageQueue`](#classnostr_1_1NostrRelay_1a0c9e582eb5dc99a49ae4bf496052f89c) 

#### `protected `[`Connection`](#classnostr_1_1Connection)` * `[`conn`](#classnostr_1_1NostrRelay_1ae456a4d62f0ede9c1dc6fc6e65f97679) 

#### `protected `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`url`](#classnostr_1_1NostrRelay_1a36f2d542488906bf120ad44190c9c0d4) 

#### `protected void `[`processQueue`](#classnostr_1_1NostrRelay_1aa03ceeb2e171b89412fbdab332205c0a)`()` 

# class `nostr::NostrPool` 

The main class for interacting with the Nostr network. Handles subscriptions, relays, and event publishing.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`NostrPool`](#classnostr_1_1NostrPool_1aa169631cafd52ff3640edc32582ffd0f)`(`[`Transport`](#classnostr_1_1Transport)` * transport,int eventStatusTimeoutSeconds)` | Create a new [NostrPool](#classnostr_1_1NostrPool)
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`subscribeMany`](#classnostr_1_1NostrPool_1ac7234da5ae1a2b83564b1fed26cf1932)`(std::initializer_list< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > urls,std::initializer_list< std::map< `[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), std::initializer_list< [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` >>> filters,`[`NostrEventCallback`](#namespacenostr_1ab0da4b375d4d7eda0bb0bf1010f381bc)` eventCallback,`[`NostrCloseCallback`](#namespacenostr_1a6247e694a00e0a5cb14ddf0929408fb5)` closeCallback,`[`NostrEOSECallback`](#namespacenostr_1a7a3b6e8f71428b2bf4195b93d35756ed)` eoseCallback)` | Subscribe to events from one or more relays 
`public void `[`publish`](#classnostr_1_1NostrPool_1ae3a75055cdf933a6ec0dbea4e4dcbc49)`(std::initializer_list< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > rs,`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * event,`[`NostrEventStatusCallback`](#namespacenostr_1a83396304773c199d3c6613e50113ac71)` statusCallback)` | Publish a signed event to one or more relays. The pool will automatically start listening to the specified relays after publishing. 
`public void `[`loop`](#classnostr_1_1NostrPool_1ab396fdccb8964a50e88dc71357682e99)`()` | Tick the [NostrPool](#classnostr_1_1NostrPool). This should be called in the main loop of your sketch
`public void `[`closeSubscription`](#classnostr_1_1NostrPool_1ab862d897213cbb3a7c39387253b2122b)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` subId)` | Close the subscription with the given ID 
`public `[`NostrRelay`](#classnostr_1_1NostrRelay)` * `[`ensureRelay`](#classnostr_1_1NostrPool_1a9f6686275d59a1556b30c58737d45c0b)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` | Ensure the connection to the specified relay is open 
`public std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`getRelays`](#classnostr_1_1NostrPool_1a87aacd8a79d84fa8a798f824ab21e16c)`()` | Get all relays that the pool is connected to 
`public void `[`disconnectRelay`](#classnostr_1_1NostrPool_1a6930e93774d6d5dba07a7172bffd3684)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` | Disconnect from the specified relay 
`public void `[`close`](#classnostr_1_1NostrPool_1a912d173034c08ea82922acf58c8acbc2)`()` | Close the pool and disconnect from every relay

## Members

#### `public inline  `[`NostrPool`](#classnostr_1_1NostrPool_1aa169631cafd52ff3640edc32582ffd0f)`(`[`Transport`](#classnostr_1_1Transport)` * transport,int eventStatusTimeoutSeconds)` 

Create a new [NostrPool](#classnostr_1_1NostrPool)
#### Parameters
* `transport` A transport object, you can get one for the [esp32](#namespacenostr_1_1esp32) platform using [nostr::esp32::ESP32Platform::getTransport()](#namespacenostr_1_1esp32_1_1ESP32Platform_1abe46d31af712a8dc70582da944ce1229) , for other platforms you need to implement the [Transport](#classnostr_1_1Transport) interface 

* `eventStatusTimeoutSeconds` The number of seconds to wait for an event status before timing out (optional)

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`subscribeMany`](#classnostr_1_1NostrPool_1ac7234da5ae1a2b83564b1fed26cf1932)`(std::initializer_list< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > urls,std::initializer_list< std::map< `[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), std::initializer_list< [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` >>> filters,`[`NostrEventCallback`](#namespacenostr_1ab0da4b375d4d7eda0bb0bf1010f381bc)` eventCallback,`[`NostrCloseCallback`](#namespacenostr_1a6247e694a00e0a5cb14ddf0929408fb5)` closeCallback,`[`NostrEOSECallback`](#namespacenostr_1a7a3b6e8f71428b2bf4195b93d35756ed)` eoseCallback)` 

Subscribe to events from one or more relays 
#### Parameters
* `urls` The relays to subscribe to 

* `filters` A list of filters for each relay 

* `eventCallback` A callback to be called when an event is received (optional) 

* `closeCallback` A callback to be called when the subscription is closed (optional) 

* `eoseCallback` A callback to be called when the stored events are exhausted (optional) 

#### Returns
The subscription ID

#### `public void `[`publish`](#classnostr_1_1NostrPool_1ae3a75055cdf933a6ec0dbea4e4dcbc49)`(std::initializer_list< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > rs,`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * event,`[`NostrEventStatusCallback`](#namespacenostr_1a83396304773c199d3c6613e50113ac71)` statusCallback)` 

Publish a signed event to one or more relays. The pool will automatically start listening to the specified relays after publishing. 
#### Parameters
* `rs` The relays to publish to 

* `event` The event to publish 

* `statusCallback` A callback to be called when the event status is known (optional)

#### `public void `[`loop`](#classnostr_1_1NostrPool_1ab396fdccb8964a50e88dc71357682e99)`()` 

Tick the [NostrPool](#classnostr_1_1NostrPool). This should be called in the main loop of your sketch

#### `public void `[`closeSubscription`](#classnostr_1_1NostrPool_1ab862d897213cbb3a7c39387253b2122b)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` subId)` 

Close the subscription with the given ID 
#### Parameters
* `subId` The subscription ID

#### `public `[`NostrRelay`](#classnostr_1_1NostrRelay)` * `[`ensureRelay`](#classnostr_1_1NostrPool_1a9f6686275d59a1556b30c58737d45c0b)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` 

Ensure the connection to the specified relay is open 
#### Parameters
* `url` The relay URL 

#### Returns
The relay object

#### `public std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`getRelays`](#classnostr_1_1NostrPool_1a87aacd8a79d84fa8a798f824ab21e16c)`()` 

Get all relays that the pool is connected to 
#### Returns
A list of relay URLs

#### `public void `[`disconnectRelay`](#classnostr_1_1NostrPool_1a6930e93774d6d5dba07a7172bffd3684)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` 

Disconnect from the specified relay 
#### Parameters
* `url` The relay URL

#### `public void `[`close`](#classnostr_1_1NostrPool_1a912d173034c08ea82922acf58c8acbc2)`()` 

Close the pool and disconnect from every relay

# class `nostr::NWCResponseCallbackBase` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> `[`onErr`](#classnostr_1_1NWCResponseCallbackBase_1a69343fb4b0962617b95fcbac5e57bf39) | 
`public unsigned long long `[`timestampSeconds`](#classnostr_1_1NWCResponseCallbackBase_1a9cc13df78749e62d5c2f967bba5e30de) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`eventId`](#classnostr_1_1NWCResponseCallbackBase_1a556e8ebe5a7eae9ff66d22d16e1dd58f) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`subId`](#classnostr_1_1NWCResponseCallbackBase_1a1522c5ed0c134503757c3155b1ea755f) | 
`public unsigned int `[`n`](#classnostr_1_1NWCResponseCallbackBase_1a3ab0eff64ab942a724e9744520f0c964) | 
`public virtual  `[`~NWCResponseCallbackBase`](#classnostr_1_1NWCResponseCallbackBase_1afe923535525c206b1c4a51b15a51ffa2)`() = default` | 
`public inline virtual void `[`call`](#classnostr_1_1NWCResponseCallbackBase_1a1f31ddae6a8248594d8c646ae20bfdd3)`(`[`Nip47`](#classnostr_1_1Nip47)` * nip47,`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * ev)` | 

## Members

#### `public std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> `[`onErr`](#classnostr_1_1NWCResponseCallbackBase_1a69343fb4b0962617b95fcbac5e57bf39) 

#### `public unsigned long long `[`timestampSeconds`](#classnostr_1_1NWCResponseCallbackBase_1a9cc13df78749e62d5c2f967bba5e30de) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`eventId`](#classnostr_1_1NWCResponseCallbackBase_1a556e8ebe5a7eae9ff66d22d16e1dd58f) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`subId`](#classnostr_1_1NWCResponseCallbackBase_1a1522c5ed0c134503757c3155b1ea755f) 

#### `public unsigned int `[`n`](#classnostr_1_1NWCResponseCallbackBase_1a3ab0eff64ab942a724e9744520f0c964) 

#### `public virtual  `[`~NWCResponseCallbackBase`](#classnostr_1_1NWCResponseCallbackBase_1afe923535525c206b1c4a51b15a51ffa2)`() = default` 

#### `public inline virtual void `[`call`](#classnostr_1_1NWCResponseCallbackBase_1a1f31ddae6a8248594d8c646ae20bfdd3)`(`[`Nip47`](#classnostr_1_1Nip47)` * nip47,`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * ev)` 

# class `nostr::NWCResponseCallback` 

```
class nostr::NWCResponseCallback
  : public nostr::NWCResponseCallbackBase
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public std::function< void(T)> `[`onRes`](#classnostr_1_1NWCResponseCallback_1aa440e86bed3377ff268ba12c67e385ed) | 
`public inline virtual void `[`call`](#classnostr_1_1NWCResponseCallback_1a2b29a2bbfb29b58b098272c90023ef5b)`(`[`Nip47`](#classnostr_1_1Nip47)` * nip47,`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * ev)` | 

## Members

#### `public std::function< void(T)> `[`onRes`](#classnostr_1_1NWCResponseCallback_1aa440e86bed3377ff268ba12c67e385ed) 

#### `public inline virtual void `[`call`](#classnostr_1_1NWCResponseCallback_1a2b29a2bbfb29b58b098272c90023ef5b)`(`[`Nip47`](#classnostr_1_1Nip47)` * nip47,`[`SignedNostrEvent`](#classnostr_1_1SignedNostrEvent)` * ev)` 

# class `nostr::NWC` 

A class to interact with the Nostr Wallet Connect services without dealing with the underlying transport

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`~NWC`](#classnostr_1_1NWC_1aa776fd16b22ee081a1ba837f7498edd2)`()` | 
`public  `[`NWC`](#classnostr_1_1NWC_1a7ce0d1f12d37b1f1217fd16240137e3b)`(`[`Transport`](#classnostr_1_1Transport)` * transport,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` nwcUrl)` | Initialize a standalone [NWC](#classnostr_1_1NWC) service 
`public void `[`loop`](#classnostr_1_1NWC_1a986c7fa8b9480303f19be584b575e611)`()` | Tick the main loop. This should be called in the [loop()](#classnostr_1_1NWC_1a986c7fa8b9480303f19be584b575e611) function of your sketch
`public void `[`close`](#classnostr_1_1NWC_1aa549023b05bdf32ecb43bf6587db9428)`()` | Close the service, after this you should throw away the [NWC](#classnostr_1_1NWC) object
`public void `[`payInvoice`](#classnostr_1_1NWC_1ad1273a9db63479d103ede6af0657c1c9)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice,unsigned long amount,std::function< void(`[`PayInvoiceResponse`](#namespacenostr_1aabe8d230016fd5f70285adbcbe77fe23))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Pay a lightning invoice 
`public void `[`multiPayInvoice`](#classnostr_1_1NWC_1aba3161e7f5badbc631e7c861c4557107)`(std::initializer_list< `[`Invoice`](#namespacenostr_1ac0f491ca12edeaa060bf89f8349d1c94)` > invoices,std::function< void(`[`MultiPayInvoiceResponse`](#namespacenostr_1a0fb78e894360c3b4167124456e18cc1b))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Pay multiple lightning invoices 
`public void `[`payKeySend`](#classnostr_1_1NWC_1a3838a879e0864c482285f4883fea997f)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` pubkey,unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` preimage,std::initializer_list< `[`TLVRecords`](#namespacenostr_1ad0f4687c761ee6988b42826d3ea717f6)` > tlv,std::function< void(`[`PayKeySendResponse`](#namespacenostr_1a70810c5e5dbfe96e16ae12fd2990e6d6))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Send a keysend payment 
`public void `[`multiPayKeySend`](#classnostr_1_1NWC_1a334c69af60cc76d8c32ba4f8fa3d621a)`(std::initializer_list< `[`KeySend`](#namespacenostr_1a2f0adf8fb06541c0734900f1f37c6a77)` > keySends,std::function< void(`[`MultiPayKeySendResponse`](#namespacenostr_1af6e0f43f7f982a4b4fe1a1c097cf9de3))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Send multiple keysend payments 
`public void `[`makeInvoice`](#classnostr_1_1NWC_1a54556399de8c19c5a3c72b3262616b9c)`(unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` description,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` descriptionHash,unsigned long expiry,std::function< void(`[`MakeInvoiceResponse`](#namespacenostr_1a784f3d1260b727625d00d370c0ac9e9f))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Create a lightning invoice 
`public void `[`lookUpPaymentHash`](#classnostr_1_1NWC_1a8c34f915eae08833ca38ca62f0811cdc)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` paymentHash,std::function< void(`[`LookUpInvoiceResponse`](#namespacenostr_1a9357541c46480c78b10dbd47531ce2d7))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Look up a payment hash 
`public void `[`lookUpInvoice`](#classnostr_1_1NWC_1a3e6bf85d69624a302cdce0db89f942b6)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice,std::function< void(`[`LookUpInvoiceResponse`](#namespacenostr_1a9357541c46480c78b10dbd47531ce2d7))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Look up an invoice 
`public void `[`listTransactions`](#classnostr_1_1NWC_1a77246d9ec254155500dfd35a3ed46f87)`(unsigned long from,unsigned long until,int limit,int offset,bool unpaid,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` type,std::function< void(`[`ListTransactionsResponse`](#namespacenostr_1a5fd7239e7510091c73ba2602f4a6bc3d))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | List transactions 
`public void `[`getBalance`](#classnostr_1_1NWC_1a55874260c4af288a620e0b19d62ea949)`(std::function< void(`[`GetBalanceResponse`](#namespacenostr_1a42570ba6e2e70160a34b1746331f5bff))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Get the balance 
`public void `[`getInfo`](#classnostr_1_1NWC_1ad646581995f7b719a14d46f26e000896)`(std::function< void(`[`GetInfoResponse`](#namespacenostr_1af3dc0dbf7fa69605d5ad971a37c5fe7f))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` | Get the info 

## Members

#### `public  `[`~NWC`](#classnostr_1_1NWC_1aa776fd16b22ee081a1ba837f7498edd2)`()` 

#### `public  `[`NWC`](#classnostr_1_1NWC_1a7ce0d1f12d37b1f1217fd16240137e3b)`(`[`Transport`](#classnostr_1_1Transport)` * transport,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` nwcUrl)` 

Initialize a standalone [NWC](#classnostr_1_1NWC) service 
#### Parameters
* `transport` A transport object, you can get one for the [esp32](#namespacenostr_1_1esp32) platform using [nostr::esp32::ESP32Platform::getTransport()](#namespacenostr_1_1esp32_1_1ESP32Platform_1abe46d31af712a8dc70582da944ce1229) , for other platforms you need to implement the [Transport](#classnostr_1_1Transport) interface 

* `nwcUrl` The [NWC](#classnostr_1_1NWC) connection URL

#### `public void `[`loop`](#classnostr_1_1NWC_1a986c7fa8b9480303f19be584b575e611)`()` 

Tick the main loop. This should be called in the [loop()](#classnostr_1_1NWC_1a986c7fa8b9480303f19be584b575e611) function of your sketch

#### `public void `[`close`](#classnostr_1_1NWC_1aa549023b05bdf32ecb43bf6587db9428)`()` 

Close the service, after this you should throw away the [NWC](#classnostr_1_1NWC) object

#### `public void `[`payInvoice`](#classnostr_1_1NWC_1ad1273a9db63479d103ede6af0657c1c9)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice,unsigned long amount,std::function< void(`[`PayInvoiceResponse`](#namespacenostr_1aabe8d230016fd5f70285adbcbe77fe23))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Pay a lightning invoice 
#### Parameters
* `invoice` The invoice to pay 

* `amount` The amount to pay, if not specified the full amount will be paid (optional) 

* `onRes` A callback that will be called when the payment is successful (optional) 

* `onErr` A callback that will be called when the payment fails (optional)

#### `public void `[`multiPayInvoice`](#classnostr_1_1NWC_1aba3161e7f5badbc631e7c861c4557107)`(std::initializer_list< `[`Invoice`](#namespacenostr_1ac0f491ca12edeaa060bf89f8349d1c94)` > invoices,std::function< void(`[`MultiPayInvoiceResponse`](#namespacenostr_1a0fb78e894360c3b4167124456e18cc1b))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Pay multiple lightning invoices 
#### Parameters
* `invoices` A list of invoices to pay 

* `onRes` A callback that will be called when the payment is successful (nb. it will be called once for each invoice) (optional) 

* `onErr` A callback that will be called when the payment fails (optional)

#### `public void `[`payKeySend`](#classnostr_1_1NWC_1a3838a879e0864c482285f4883fea997f)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` pubkey,unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` preimage,std::initializer_list< `[`TLVRecords`](#namespacenostr_1ad0f4687c761ee6988b42826d3ea717f6)` > tlv,std::function< void(`[`PayKeySendResponse`](#namespacenostr_1a70810c5e5dbfe96e16ae12fd2990e6d6))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Send a keysend payment 
#### Parameters
* `pubkey` The pubkey to send to 

* `amount` The amount to send 

* `preimage` The preimage to use (optional) 

* `tlv` A list of TLV records to include (optional) 

* `onRes` A callback that will be called when the payment is successful (optional) 

* `onErr` A callback that will be called when the payment fails (optional)

#### `public void `[`multiPayKeySend`](#classnostr_1_1NWC_1a334c69af60cc76d8c32ba4f8fa3d621a)`(std::initializer_list< `[`KeySend`](#namespacenostr_1a2f0adf8fb06541c0734900f1f37c6a77)` > keySends,std::function< void(`[`MultiPayKeySendResponse`](#namespacenostr_1af6e0f43f7f982a4b4fe1a1c097cf9de3))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Send multiple keysend payments 
#### Parameters
* `keySends` A list of keysend payments to send 

* `onRes` A callback that will be called when the payment is successful (nb. it will be called once for each keysend) (optional) 

* `onErr` A callback that will be called when the payment fails (optional)

#### `public void `[`makeInvoice`](#classnostr_1_1NWC_1a54556399de8c19c5a3c72b3262616b9c)`(unsigned long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` description,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` descriptionHash,unsigned long expiry,std::function< void(`[`MakeInvoiceResponse`](#namespacenostr_1a784f3d1260b727625d00d370c0ac9e9f))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Create a lightning invoice 
#### Parameters
* `amount` The amount to invoice 

* `description` A description of the invoice (optional) 

* `descriptionHash` A hash of the description (optional) 

* `expiry` The expiry time of the invoice in seconds (optional) 

* `onRes` A callback that will be called when the invoice is created (optional) 

* `onErr` A callback that will be called when the invoice creation fails (optional)

#### `public void `[`lookUpPaymentHash`](#classnostr_1_1NWC_1a8c34f915eae08833ca38ca62f0811cdc)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` paymentHash,std::function< void(`[`LookUpInvoiceResponse`](#namespacenostr_1a9357541c46480c78b10dbd47531ce2d7))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Look up a payment hash 
#### Parameters
* `paymentHash` The payment hash to look up 

* `onRes` A callback that will be called when the payment is found (optional) 

* `onErr` A callback that will be called when the payment is not found (optional)

#### `public void `[`lookUpInvoice`](#classnostr_1_1NWC_1a3e6bf85d69624a302cdce0db89f942b6)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` invoice,std::function< void(`[`LookUpInvoiceResponse`](#namespacenostr_1a9357541c46480c78b10dbd47531ce2d7))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Look up an invoice 
#### Parameters
* `invoice` The invoice to look up 

* `onRes` A callback that will be called when the invoice is found (optional) 

* `onErr` A callback that will be called when the invoice is not found (optional)

#### `public void `[`listTransactions`](#classnostr_1_1NWC_1a77246d9ec254155500dfd35a3ed46f87)`(unsigned long from,unsigned long until,int limit,int offset,bool unpaid,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` type,std::function< void(`[`ListTransactionsResponse`](#namespacenostr_1a5fd7239e7510091c73ba2602f4a6bc3d))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

List transactions 
#### Parameters
* `from` The start time of the transactions to list (optional) 

* `until` The end time of the transactions to list (optional) 

* `limit` The maximum number of transactions to list (optional) 

* `offset` The offset of the transactions to list (optional) 

* `unpaid` Whether to list only unpaid transactions (optional) 

* `type` The type of transactions to list (optional) 

* `onRes` A callback that will be called when the transactions are listed (optional) 

* `onErr` A callback that will be called when the transactions listing fails (optional)

#### `public void `[`getBalance`](#classnostr_1_1NWC_1a55874260c4af288a620e0b19d62ea949)`(std::function< void(`[`GetBalanceResponse`](#namespacenostr_1a42570ba6e2e70160a34b1746331f5bff))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Get the balance 
#### Parameters
* `onRes` A callback that will be called when the balance is retrieved (optional) 

* `onErr` A callback that will be called when the balance retrieval fails (optional)

#### `public void `[`getInfo`](#classnostr_1_1NWC_1ad646581995f7b719a14d46f26e000896)`(std::function< void(`[`GetInfoResponse`](#namespacenostr_1af3dc0dbf7fa69605d5ad971a37c5fe7f))`> onRes,std::function< void(`[`NostrString](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626), [NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> onErr)` 

Get the info 
#### Parameters
* `onRes` A callback that will be called when the info is retrieved (optional) 

* `onErr` A callback that will be called when the info retrieval fails (optional)

# class `nostr::Connection` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public virtual void `[`addMessageListener`](#classnostr_1_1Connection_1a09f2f3d42f8b4143dc616207afa7cab3)`(std::function< void(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> listener)` | 
`public void `[`send`](#classnostr_1_1Connection_1ac5de060192a611ec634b1123c6ce7787)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` message)` | 
`public void `[`disconnect`](#classnostr_1_1Connection_1a0de18a7197c81db8ac43036f6dcfc4ff)`()` | 
`public virtual  `[`~Connection`](#classnostr_1_1Connection_1a8b3ae539c84b9de24c5db2071885b2c9)`() = default` | 
`public void `[`loop`](#classnostr_1_1Connection_1aee2be02ce75a3bb2a15306212e77b382)`()` | 
`public bool `[`isReady`](#classnostr_1_1Connection_1af780de347728124fc0664614f100c604)`()` | 

## Members

#### `public virtual void `[`addMessageListener`](#classnostr_1_1Connection_1a09f2f3d42f8b4143dc616207afa7cab3)`(std::function< void(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> listener)` 

#### `public void `[`send`](#classnostr_1_1Connection_1ac5de060192a611ec634b1123c6ce7787)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` message)` 

#### `public void `[`disconnect`](#classnostr_1_1Connection_1a0de18a7197c81db8ac43036f6dcfc4ff)`()` 

#### `public virtual  `[`~Connection`](#classnostr_1_1Connection_1a8b3ae539c84b9de24c5db2071885b2c9)`() = default` 

#### `public void `[`loop`](#classnostr_1_1Connection_1aee2be02ce75a3bb2a15306212e77b382)`()` 

#### `public bool `[`isReady`](#classnostr_1_1Connection_1af780de347728124fc0664614f100c604)`()` 

# class `nostr::Transport` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getInvoiceFromLNAddr`](#classnostr_1_1Transport_1a42bedd0b5a19ddfc1ae5c02cd14edf14)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` addr,unsigned long long amountMSats,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` comment)` | 
`public `[`Connection`](#classnostr_1_1Connection)` * `[`connect`](#classnostr_1_1Transport_1a0c5e942cdc1befc66aae956d94469efd)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` | 
`public virtual  `[`~Transport`](#classnostr_1_1Transport_1af063ade88c53338ad4d7489c59b39625)`() = default` | 
`public  `[`Transport`](#classnostr_1_1Transport_1a87434e88eff5bc156ce6ef4a0f9e4d41)`() = default` | 
`public void `[`disconnect`](#classnostr_1_1Transport_1a574027b0ae5f5d1f67c64396ec8febc0)`(`[`Connection`](#classnostr_1_1Connection)` * conn)` | 
`public bool `[`isReady`](#classnostr_1_1Transport_1a277eac950191c1f87450d712c5bf6ec9)`()` | 
`public void `[`close`](#classnostr_1_1Transport_1a0afe71e02773d9a026474372442e674a)`()` | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getInvoiceFromLNAddr`](#classnostr_1_1Transport_1a42bedd0b5a19ddfc1ae5c02cd14edf14)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` addr,unsigned long long amountMSats,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` comment)` 

#### `public `[`Connection`](#classnostr_1_1Connection)` * `[`connect`](#classnostr_1_1Transport_1a0c5e942cdc1befc66aae956d94469efd)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` 

#### `public virtual  `[`~Transport`](#classnostr_1_1Transport_1af063ade88c53338ad4d7489c59b39625)`() = default` 

#### `public  `[`Transport`](#classnostr_1_1Transport_1a87434e88eff5bc156ce6ef4a0f9e4d41)`() = default` 

#### `public void `[`disconnect`](#classnostr_1_1Transport_1a574027b0ae5f5d1f67c64396ec8febc0)`(`[`Connection`](#classnostr_1_1Connection)` * conn)` 

#### `public bool `[`isReady`](#classnostr_1_1Transport_1a277eac950191c1f87450d712c5bf6ec9)`()` 

#### `public void `[`close`](#classnostr_1_1Transport_1a0afe71e02773d9a026474372442e674a)`()` 

# class `nostr::Utils` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------

## Members

# struct `nostr::s_Invoice` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__Invoice_1a0cf0cb757c30335450f87487fd8f0d4c) | 
`public unsigned long `[`amount`](#structnostr_1_1s__Invoice_1a570f5370ade3dffa6e686e4b834810cf) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__Invoice_1a0cf0cb757c30335450f87487fd8f0d4c) 

#### `public unsigned long `[`amount`](#structnostr_1_1s__Invoice_1a570f5370ade3dffa6e686e4b834810cf) 

# struct `nostr::s_TLVRecords` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__TLVRecords_1ae70d4a2a9a54e2e241d72216664dcd9c) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`value`](#structnostr_1_1s__TLVRecords_1a76997f69bae788ce62355110121c3cca) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__TLVRecords_1ae70d4a2a9a54e2e241d72216664dcd9c) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`value`](#structnostr_1_1s__TLVRecords_1a76997f69bae788ce62355110121c3cca) 

# struct `nostr::s_KeySend` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`pubkey`](#structnostr_1_1s__KeySend_1a885a1dc2902876ccf61b2023a416e51c) | 
`public unsigned long `[`amount`](#structnostr_1_1s__KeySend_1a579d6e66f6b605aec6c65357c58c0c3b) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__KeySend_1a063d8accd6247e37795607e320c65896) | 
`public std::initializer_list< `[`TLVRecords`](#namespacenostr_1ad0f4687c761ee6988b42826d3ea717f6)` > `[`tlv`](#structnostr_1_1s__KeySend_1a3a8b4e86ea8de97c704e9ed2b0df2c54) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`pubkey`](#structnostr_1_1s__KeySend_1a885a1dc2902876ccf61b2023a416e51c) 

#### `public unsigned long `[`amount`](#structnostr_1_1s__KeySend_1a579d6e66f6b605aec6c65357c58c0c3b) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__KeySend_1a063d8accd6247e37795607e320c65896) 

#### `public std::initializer_list< `[`TLVRecords`](#namespacenostr_1ad0f4687c761ee6988b42826d3ea717f6)` > `[`tlv`](#structnostr_1_1s__KeySend_1a3a8b4e86ea8de97c704e9ed2b0df2c54) 

# struct `nostr::s_PayInvoiceResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__PayInvoiceResponse_1a8e8e2ad8e7dd44c1ba197c555efa0742) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__PayInvoiceResponse_1a8e8e2ad8e7dd44c1ba197c555efa0742) 

# struct `nostr::s_MultiPayInvoiceResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__MultiPayInvoiceResponse_1afb1495a862c7d5322d8a630c5ff54df6) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`d`](#structnostr_1_1s__MultiPayInvoiceResponse_1a512ff06432c3810adbf31889d031d98e) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__MultiPayInvoiceResponse_1afb1495a862c7d5322d8a630c5ff54df6) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`d`](#structnostr_1_1s__MultiPayInvoiceResponse_1a512ff06432c3810adbf31889d031d98e) 

# struct `nostr::s_PayKeySendResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__PayKeySendResponse_1a480aef63eec69818a9135b08150e6285) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__PayKeySendResponse_1a480aef63eec69818a9135b08150e6285) 

# struct `nostr::s_MultiPayKeySendResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__MultiPayKeySendResponse_1acb8859c0a2c2ddffa22258513c26837b) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`d`](#structnostr_1_1s__MultiPayKeySendResponse_1aa4a2a52842a2987b436b24afa6efd018) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__MultiPayKeySendResponse_1acb8859c0a2c2ddffa22258513c26837b) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`d`](#structnostr_1_1s__MultiPayKeySendResponse_1aa4a2a52842a2987b436b24afa6efd018) 

# struct `nostr::s_MakeInvoiceResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__MakeInvoiceResponse_1a1c2af5e4a031832157918c116d282573) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__MakeInvoiceResponse_1a75edbc8251ce0d65c03af38b47926f55) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`description`](#structnostr_1_1s__MakeInvoiceResponse_1aae625065fb44b02e5883e5297a065bee) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`descriptionHash`](#structnostr_1_1s__MakeInvoiceResponse_1ab01309d75ed6af2acfddffa7c184e36f) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__MakeInvoiceResponse_1a1c6266a0cac33e3e0ea0c9996c2f4119) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`paymentHash`](#structnostr_1_1s__MakeInvoiceResponse_1a6e43d10fb27320b4d619608ec1c4b5f3) | 
`public unsigned long long `[`amount`](#structnostr_1_1s__MakeInvoiceResponse_1ae219cf962ee5d75b28553f2d4738aaa0) | 
`public unsigned long long `[`feesPaid`](#structnostr_1_1s__MakeInvoiceResponse_1a8a605d9c39fb32cbe4eb788a349a8956) | 
`public unsigned long long `[`createdAt`](#structnostr_1_1s__MakeInvoiceResponse_1ad19ebeaedf39995483e4017b9bab3a0f) | 
`public unsigned long long `[`expiresAt`](#structnostr_1_1s__MakeInvoiceResponse_1a05a1a18f8513810f4be524f7492989a6) | 
`public JsonObject `[`metadata`](#structnostr_1_1s__MakeInvoiceResponse_1ac16c2e27e0f00331c28f7221340570ff) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__MakeInvoiceResponse_1a1c2af5e4a031832157918c116d282573) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__MakeInvoiceResponse_1a75edbc8251ce0d65c03af38b47926f55) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`description`](#structnostr_1_1s__MakeInvoiceResponse_1aae625065fb44b02e5883e5297a065bee) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`descriptionHash`](#structnostr_1_1s__MakeInvoiceResponse_1ab01309d75ed6af2acfddffa7c184e36f) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__MakeInvoiceResponse_1a1c6266a0cac33e3e0ea0c9996c2f4119) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`paymentHash`](#structnostr_1_1s__MakeInvoiceResponse_1a6e43d10fb27320b4d619608ec1c4b5f3) 

#### `public unsigned long long `[`amount`](#structnostr_1_1s__MakeInvoiceResponse_1ae219cf962ee5d75b28553f2d4738aaa0) 

#### `public unsigned long long `[`feesPaid`](#structnostr_1_1s__MakeInvoiceResponse_1a8a605d9c39fb32cbe4eb788a349a8956) 

#### `public unsigned long long `[`createdAt`](#structnostr_1_1s__MakeInvoiceResponse_1ad19ebeaedf39995483e4017b9bab3a0f) 

#### `public unsigned long long `[`expiresAt`](#structnostr_1_1s__MakeInvoiceResponse_1a05a1a18f8513810f4be524f7492989a6) 

#### `public JsonObject `[`metadata`](#structnostr_1_1s__MakeInvoiceResponse_1ac16c2e27e0f00331c28f7221340570ff) 

# struct `nostr::s_LookUpInvoiceResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__LookUpInvoiceResponse_1a2f09c87cb594d572f34c586b2df46cc6) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__LookUpInvoiceResponse_1afdb026a867244c8242f76a77634e68d8) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`description`](#structnostr_1_1s__LookUpInvoiceResponse_1a4718ebc0fa29823f23eec1d8ee258e31) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`descriptionHash`](#structnostr_1_1s__LookUpInvoiceResponse_1aa14473941cfc6ebeb53581d595cfd077) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__LookUpInvoiceResponse_1aa2e248d0bf689bd66fec41e665518356) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`paymentHash`](#structnostr_1_1s__LookUpInvoiceResponse_1a22f4d51e2a556934ae3172c333fbfa29) | 
`public unsigned long long `[`amount`](#structnostr_1_1s__LookUpInvoiceResponse_1aadf7a1605324725038c6d2bfd7db6959) | 
`public unsigned long long `[`feesPaid`](#structnostr_1_1s__LookUpInvoiceResponse_1acb4d6268a52b5d8262688156fd7c6232) | 
`public unsigned long long `[`createdAt`](#structnostr_1_1s__LookUpInvoiceResponse_1a1d9c0ef9a426afa1c861118d65851d3d) | 
`public unsigned long long `[`expiresAt`](#structnostr_1_1s__LookUpInvoiceResponse_1a765e80a64313dee017711bfbb680a45b) | 
`public unsigned long long `[`settledAt`](#structnostr_1_1s__LookUpInvoiceResponse_1a1db3d3727b086a8a531177747fd55bc8) | 
`public JsonObject `[`metadata`](#structnostr_1_1s__LookUpInvoiceResponse_1ac8cf33fd35ffefc4be14e44d18eef87e) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__LookUpInvoiceResponse_1a2f09c87cb594d572f34c586b2df46cc6) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__LookUpInvoiceResponse_1afdb026a867244c8242f76a77634e68d8) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`description`](#structnostr_1_1s__LookUpInvoiceResponse_1a4718ebc0fa29823f23eec1d8ee258e31) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`descriptionHash`](#structnostr_1_1s__LookUpInvoiceResponse_1aa14473941cfc6ebeb53581d595cfd077) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__LookUpInvoiceResponse_1aa2e248d0bf689bd66fec41e665518356) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`paymentHash`](#structnostr_1_1s__LookUpInvoiceResponse_1a22f4d51e2a556934ae3172c333fbfa29) 

#### `public unsigned long long `[`amount`](#structnostr_1_1s__LookUpInvoiceResponse_1aadf7a1605324725038c6d2bfd7db6959) 

#### `public unsigned long long `[`feesPaid`](#structnostr_1_1s__LookUpInvoiceResponse_1acb4d6268a52b5d8262688156fd7c6232) 

#### `public unsigned long long `[`createdAt`](#structnostr_1_1s__LookUpInvoiceResponse_1a1d9c0ef9a426afa1c861118d65851d3d) 

#### `public unsigned long long `[`expiresAt`](#structnostr_1_1s__LookUpInvoiceResponse_1a765e80a64313dee017711bfbb680a45b) 

#### `public unsigned long long `[`settledAt`](#structnostr_1_1s__LookUpInvoiceResponse_1a1db3d3727b086a8a531177747fd55bc8) 

#### `public JsonObject `[`metadata`](#structnostr_1_1s__LookUpInvoiceResponse_1ac8cf33fd35ffefc4be14e44d18eef87e) 

# struct `nostr::s_Transaction` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__Transaction_1a4d7ee082010c57d95e5916f4b0de0c65) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__Transaction_1aecfced2e600a9c5ab15c8522ae48599f) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`description`](#structnostr_1_1s__Transaction_1a549be99869016e36a2a52eeae213c8b6) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`descriptionHash`](#structnostr_1_1s__Transaction_1a5ad99841dd85b5b19d29e1f86a07c1b6) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__Transaction_1adfe56e48af5a808da3d450968f089427) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`paymentHash`](#structnostr_1_1s__Transaction_1a70d6b59067a9b3586e62da14415f5345) | 
`public unsigned long long `[`amount`](#structnostr_1_1s__Transaction_1a85f76ec98f03492272430c5b918bcbfc) | 
`public unsigned long long `[`feesPaid`](#structnostr_1_1s__Transaction_1ab2ca96696990f5ca3d1cfc020ac85663) | 
`public unsigned long long `[`createdAt`](#structnostr_1_1s__Transaction_1ac4520e36db73ca34ffbef5cfd1f5112c) | 
`public unsigned long long `[`expiresAt`](#structnostr_1_1s__Transaction_1a1d0cacb6699476e43f736c9f5c08493e) | 
`public unsigned long long `[`settledAt`](#structnostr_1_1s__Transaction_1aec0504cc1a5f83f73086e636de478637) | 
`public JsonObject `[`metadata`](#structnostr_1_1s__Transaction_1a084822125bb2c2622a7af6eb2dc4d7df) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`type`](#structnostr_1_1s__Transaction_1a4d7ee082010c57d95e5916f4b0de0c65) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`invoice`](#structnostr_1_1s__Transaction_1aecfced2e600a9c5ab15c8522ae48599f) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`description`](#structnostr_1_1s__Transaction_1a549be99869016e36a2a52eeae213c8b6) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`descriptionHash`](#structnostr_1_1s__Transaction_1a5ad99841dd85b5b19d29e1f86a07c1b6) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`preimage`](#structnostr_1_1s__Transaction_1adfe56e48af5a808da3d450968f089427) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`paymentHash`](#structnostr_1_1s__Transaction_1a70d6b59067a9b3586e62da14415f5345) 

#### `public unsigned long long `[`amount`](#structnostr_1_1s__Transaction_1a85f76ec98f03492272430c5b918bcbfc) 

#### `public unsigned long long `[`feesPaid`](#structnostr_1_1s__Transaction_1ab2ca96696990f5ca3d1cfc020ac85663) 

#### `public unsigned long long `[`createdAt`](#structnostr_1_1s__Transaction_1ac4520e36db73ca34ffbef5cfd1f5112c) 

#### `public unsigned long long `[`expiresAt`](#structnostr_1_1s__Transaction_1a1d0cacb6699476e43f736c9f5c08493e) 

#### `public unsigned long long `[`settledAt`](#structnostr_1_1s__Transaction_1aec0504cc1a5f83f73086e636de478637) 

#### `public JsonObject `[`metadata`](#structnostr_1_1s__Transaction_1a084822125bb2c2622a7af6eb2dc4d7df) 

# struct `nostr::s_ListTransactionsResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public std::vector< `[`Transaction`](#namespacenostr_1a324c9ccba9c6ebcc1753c63eaa7dca47)` > `[`transactions`](#structnostr_1_1s__ListTransactionsResponse_1a7fd6e179ba28b6f24b589921083e9697) | 

## Members

#### `public std::vector< `[`Transaction`](#namespacenostr_1a324c9ccba9c6ebcc1753c63eaa7dca47)` > `[`transactions`](#structnostr_1_1s__ListTransactionsResponse_1a7fd6e179ba28b6f24b589921083e9697) 

# struct `nostr::s_GetBalanceResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public unsigned long long `[`balance`](#structnostr_1_1s__GetBalanceResponse_1a2c8b0f2daac74e6d92a9948489c762e5) | 

## Members

#### `public unsigned long long `[`balance`](#structnostr_1_1s__GetBalanceResponse_1a2c8b0f2daac74e6d92a9948489c762e5) 

# struct `nostr::s_GetInfoResponse` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`alias`](#structnostr_1_1s__GetInfoResponse_1a754539daf350acbc6a2dd836f398330a) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`color`](#structnostr_1_1s__GetInfoResponse_1a8953b5c92c1a5691dbad1cc3c9619734) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`pubkey`](#structnostr_1_1s__GetInfoResponse_1a7fa2f8a9c91f209567ffe1d7549ac16b) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`network`](#structnostr_1_1s__GetInfoResponse_1ab731456e15cf89796232608fd69d81ec) | 
`public unsigned long long `[`blockHeight`](#structnostr_1_1s__GetInfoResponse_1a67d48a9483d8d5e7a57bce58bdad19d0) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`blockHash`](#structnostr_1_1s__GetInfoResponse_1a2cad02f859361db1c0c96044429a6c7d) | 
`public std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`methods`](#structnostr_1_1s__GetInfoResponse_1a8d8341c772deb30a61202afbd19b6e1d) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`alias`](#structnostr_1_1s__GetInfoResponse_1a754539daf350acbc6a2dd836f398330a) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`color`](#structnostr_1_1s__GetInfoResponse_1a8953b5c92c1a5691dbad1cc3c9619734) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`pubkey`](#structnostr_1_1s__GetInfoResponse_1a7fa2f8a9c91f209567ffe1d7549ac16b) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`network`](#structnostr_1_1s__GetInfoResponse_1ab731456e15cf89796232608fd69d81ec) 

#### `public unsigned long long `[`blockHeight`](#structnostr_1_1s__GetInfoResponse_1a67d48a9483d8d5e7a57bce58bdad19d0) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`blockHash`](#structnostr_1_1s__GetInfoResponse_1a2cad02f859361db1c0c96044429a6c7d) 

#### `public std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`methods`](#structnostr_1_1s__GetInfoResponse_1a8d8341c772deb30a61202afbd19b6e1d) 

# struct `nostr::s_NWCData` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`relay`](#structnostr_1_1s__NWCData_1a70082c745e79ea268c3c64c327887460) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`pubkey`](#structnostr_1_1s__NWCData_1a33f9e8b8f85c7f6e54be23d4fd4a3b36) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`secret`](#structnostr_1_1s__NWCData_1af3225681a304d0b769be2298817dd4bf) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`relay`](#structnostr_1_1s__NWCData_1a70082c745e79ea268c3c64c327887460) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`pubkey`](#structnostr_1_1s__NWCData_1a33f9e8b8f85c7f6e54be23d4fd4a3b36) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`secret`](#structnostr_1_1s__NWCData_1af3225681a304d0b769be2298817dd4bf) 

# struct `nostr::s_NostrEventTag` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`key`](#structnostr_1_1s__NostrEventTag_1a95e8590f64380a02483b6f459be3c3ad) | 
`public std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`value`](#structnostr_1_1s__NostrEventTag_1a7be407635f4384230bf90d628dc74148) | 

## Members

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`key`](#structnostr_1_1s__NostrEventTag_1a95e8590f64380a02483b6f459be3c3ad) 

#### `public std::vector< `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` > `[`value`](#structnostr_1_1s__NostrEventTag_1a7be407635f4384230bf90d628dc74148) 

# struct `nostr::s_EventStatusCallbackEntry` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`NostrEventStatusCallback`](#namespacenostr_1a83396304773c199d3c6613e50113ac71)` `[`statusCallback`](#structnostr_1_1s__EventStatusCallbackEntry_1a2ed730136d5f326a4e2591fa3ed33eb6) | 
`public long long `[`timestampSeconds`](#structnostr_1_1s__EventStatusCallbackEntry_1a28d65d5353e527cdc50053bf10561625) | 
`public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`eventId`](#structnostr_1_1s__EventStatusCallbackEntry_1a8dacb9090776437aa743fd5b916777cb) | 

## Members

#### `public `[`NostrEventStatusCallback`](#namespacenostr_1a83396304773c199d3c6613e50113ac71)` `[`statusCallback`](#structnostr_1_1s__EventStatusCallbackEntry_1a2ed730136d5f326a4e2591fa3ed33eb6) 

#### `public long long `[`timestampSeconds`](#structnostr_1_1s__EventStatusCallbackEntry_1a28d65d5353e527cdc50053bf10561625) 

#### `public `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`eventId`](#structnostr_1_1s__EventStatusCallbackEntry_1a8dacb9090776437aa743fd5b916777cb) 

# namespace `nostr::esp32` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`nostr::esp32::ESP32Connection`](#classnostr_1_1esp32_1_1ESP32Connection) | 
`class `[`nostr::esp32::ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport) | 

# class `nostr::esp32::ESP32Connection` 

```
class nostr::esp32::ESP32Connection
  : public nostr::Connection
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public virtual void `[`send`](#classnostr_1_1esp32_1_1ESP32Connection_1a57eb5e58ef7218c13b79f08b6bb87a86)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` message)` | 
`public virtual void `[`disconnect`](#classnostr_1_1esp32_1_1ESP32Connection_1a8a21ab9c9baabe361305c97f5d7b7253)`()` | 
`public virtual void `[`loop`](#classnostr_1_1esp32_1_1ESP32Connection_1ac0bcd94ae63bdb92da45f8fee3b55ac2)`()` | 
`public virtual bool `[`isReady`](#classnostr_1_1esp32_1_1ESP32Connection_1a69db2e6817533bffc6879915b6d8c87e)`()` | 
`public virtual void `[`addMessageListener`](#classnostr_1_1esp32_1_1ESP32Connection_1a70bfc236b0189524d7d5965cba18cfc1)`(std::function< void(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> listener)` | 
`public  `[`~ESP32Connection`](#classnostr_1_1esp32_1_1ESP32Connection_1a5bcfe4c8e2b1fe994f89cd8290c53dab)`()` | 
`protected  `[`ESP32Connection`](#classnostr_1_1esp32_1_1ESP32Connection_1ac99b6416b029fd2d3f86b21423c15b64)`(`[`ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport)` * transport,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` | 

## Members

#### `public virtual void `[`send`](#classnostr_1_1esp32_1_1ESP32Connection_1a57eb5e58ef7218c13b79f08b6bb87a86)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` message)` 

#### `public virtual void `[`disconnect`](#classnostr_1_1esp32_1_1ESP32Connection_1a8a21ab9c9baabe361305c97f5d7b7253)`()` 

#### `public virtual void `[`loop`](#classnostr_1_1esp32_1_1ESP32Connection_1ac0bcd94ae63bdb92da45f8fee3b55ac2)`()` 

#### `public virtual bool `[`isReady`](#classnostr_1_1esp32_1_1ESP32Connection_1a69db2e6817533bffc6879915b6d8c87e)`()` 

#### `public virtual void `[`addMessageListener`](#classnostr_1_1esp32_1_1ESP32Connection_1a70bfc236b0189524d7d5965cba18cfc1)`(std::function< void(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626))`> listener)` 

#### `public  `[`~ESP32Connection`](#classnostr_1_1esp32_1_1ESP32Connection_1a5bcfe4c8e2b1fe994f89cd8290c53dab)`()` 

#### `protected  `[`ESP32Connection`](#classnostr_1_1esp32_1_1ESP32Connection_1ac99b6416b029fd2d3f86b21423c15b64)`(`[`ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport)` * transport,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` 

# class `nostr::esp32::ESP32Transport` 

```
class nostr::esp32::ESP32Transport
  : public nostr::Transport
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public virtual `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getInvoiceFromLNAddr`](#classnostr_1_1esp32_1_1ESP32Transport_1a4adb02ec58bdf74733f6b4cc812b99cc)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` addr,unsigned long long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` comment)` | 
`public virtual `[`Connection`](#classnostr_1_1Connection)` * `[`connect`](#classnostr_1_1esp32_1_1ESP32Transport_1aa0375d4e6040153c0aab9a02f20d3512)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` | 
`public virtual void `[`close`](#classnostr_1_1esp32_1_1ESP32Transport_1aca0ad35cfb8de39700eb79177f9b485b)`()` | 
`public virtual void `[`disconnect`](#classnostr_1_1esp32_1_1ESP32Transport_1a32171ade3f194e9c868d8b4e0bd5654c)`(`[`Connection`](#classnostr_1_1Connection)` * conn)` | 
`public  `[`~ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport_1a69dbdfebd4d87498d90a8816294acd0b)`()` | 
`public  `[`ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport_1a442863628a576f577e34f102ad5bb966)`()` | 
`public virtual bool `[`isReady`](#classnostr_1_1esp32_1_1ESP32Transport_1a23d9f1794ca60bbfa1102947081cfcc2)`()` | 

## Members

#### `public virtual `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` `[`getInvoiceFromLNAddr`](#classnostr_1_1esp32_1_1ESP32Transport_1a4adb02ec58bdf74733f6b4cc812b99cc)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` addr,unsigned long long amount,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` comment)` 

#### `public virtual `[`Connection`](#classnostr_1_1Connection)` * `[`connect`](#classnostr_1_1esp32_1_1ESP32Transport_1aa0375d4e6040153c0aab9a02f20d3512)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` url)` 

#### `public virtual void `[`close`](#classnostr_1_1esp32_1_1ESP32Transport_1aca0ad35cfb8de39700eb79177f9b485b)`()` 

#### `public virtual void `[`disconnect`](#classnostr_1_1esp32_1_1ESP32Transport_1a32171ade3f194e9c868d8b4e0bd5654c)`(`[`Connection`](#classnostr_1_1Connection)` * conn)` 

#### `public  `[`~ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport_1a69dbdfebd4d87498d90a8816294acd0b)`()` 

#### `public  `[`ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport_1a442863628a576f577e34f102ad5bb966)`()` 

#### `public virtual bool `[`isReady`](#classnostr_1_1esp32_1_1ESP32Transport_1a23d9f1794ca60bbfa1102947081cfcc2)`()` 

# namespace `nostr::esp32::ESP32Platform` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline unsigned long `[`getUnixTimestamp`](#namespacenostr_1_1esp32_1_1ESP32Platform_1a5faabbc42497acaa66f7b10f2a9d9a4b)`()`            | 
`public inline long int `[`getRealRandom`](#namespacenostr_1_1esp32_1_1ESP32Platform_1ae8019dd5e74873268e638516fa73b50e)`(long int min,long int max)`            | 
`public inline void `[`serialLogger`](#namespacenostr_1_1esp32_1_1ESP32Platform_1abdc5644c7c3ca147eb184fc1f7b0cb49)`(const `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & str)`            | 
`public inline void `[`initWifi`](#namespacenostr_1_1esp32_1_1ESP32Platform_1aef2b4ec9b016229f24baa98be113baaa)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` ssid,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` passphrase,int channel)`            | Initialize the WiFi connection
`public inline void `[`initTime`](#namespacenostr_1_1esp32_1_1ESP32Platform_1add8d06cbe3a3b96d7d3c8edd90d99d3c)`(const char * ntpServer,long gmtOffset_sec,int daylightOffset_sec)`            | Initialize the time service
`public inline void `[`initNostr`](#namespacenostr_1_1esp32_1_1ESP32Platform_1af10d1ee01a3830f608b316b4770072fb)`(bool withLogger)`            | Initialize platform specific code for the nostr library
`public inline `[`ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport)` * `[`getTransport`](#namespacenostr_1_1esp32_1_1ESP32Platform_1abe46d31af712a8dc70582da944ce1229)`()`            | Get a platform specific transport

## Members

#### `public inline unsigned long `[`getUnixTimestamp`](#namespacenostr_1_1esp32_1_1ESP32Platform_1a5faabbc42497acaa66f7b10f2a9d9a4b)`()` 

#### `public inline long int `[`getRealRandom`](#namespacenostr_1_1esp32_1_1ESP32Platform_1ae8019dd5e74873268e638516fa73b50e)`(long int min,long int max)` 

#### `public inline void `[`serialLogger`](#namespacenostr_1_1esp32_1_1ESP32Platform_1abdc5644c7c3ca147eb184fc1f7b0cb49)`(const `[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` & str)` 

#### `public inline void `[`initWifi`](#namespacenostr_1_1esp32_1_1ESP32Platform_1aef2b4ec9b016229f24baa98be113baaa)`(`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` ssid,`[`NostrString`](#NostrString_8h_1a51a5cc014ebe5eb0fda73fcf0e09b626)` passphrase,int channel)` 

Initialize the WiFi connection

#### `public inline void `[`initTime`](#namespacenostr_1_1esp32_1_1ESP32Platform_1add8d06cbe3a3b96d7d3c8edd90d99d3c)`(const char * ntpServer,long gmtOffset_sec,int daylightOffset_sec)` 

Initialize the time service

#### `public inline void `[`initNostr`](#namespacenostr_1_1esp32_1_1ESP32Platform_1af10d1ee01a3830f608b316b4770072fb)`(bool withLogger)` 

Initialize platform specific code for the nostr library

#### `public inline `[`ESP32Transport`](#classnostr_1_1esp32_1_1ESP32Transport)` * `[`getTransport`](#namespacenostr_1_1esp32_1_1ESP32Platform_1abe46d31af712a8dc70582da944ce1229)`()` 

Get a platform specific transport

# struct `AES_ctx` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public uint8_t `[`RoundKey`](#structAES__ctx_1aa475fe62fd5774891c7a42596f6d9c2e) | 
`public uint8_t `[`Iv`](#structAES__ctx_1a1fa4921f7eeaca82eb974899edcdef3a) | 

## Members

#### `public uint8_t `[`RoundKey`](#structAES__ctx_1aa475fe62fd5774891c7a42596f6d9c2e) 

#### `public uint8_t `[`Iv`](#structAES__ctx_1a1fa4921f7eeaca82eb974899edcdef3a) 

Generated by [Moxygen](https://sourcey.com/moxygen)