# Arduino Nostr Library

Nostr: A simple, open protocol that enables a truly censorship-resistant and global social network.

This library gives the ability to interact with Nostr relays with Arduino microcontrollers.

## Features

+ NostrEvent - For creation of NIP01 and NIP04 notes
+ NostrRelayManager - For management of multiple relays and message broadcasting and receiving
+ NostrQueueProcessor - For queueing up relay messages
+ NostrRequestOptions - For creating a request object that can be sent to relays

+ Send notes and private messages with support for NIP01 kind 1 and NIP04 kind 4.
+ Receieve and decrypt NIP04 messages.
+ Broadcast messages to multiple relays with the ability to specify custom callbacks for NIP01, NIP04, and OK messages.
+ Add messages to a queue for broadcasting once relays are connected.
+ Subscribe to relay events using REQ messages.
+ Specify a minimum number of relays required for broadcasting and specify a timeout to ensure messages are broadcast even if the minimum number of relays is not reached.

## Usage

See the example scripts in `examples/` for simple examples of how to use this library.