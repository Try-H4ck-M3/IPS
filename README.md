# IPS - Intrusion Prevention System

## Features

### Rate Limiting

Rate limiting helps protect your system from excessive requests by:
- Tracking requests per IP address
- Banning IPs that exceed the configured threshold
- Automatically unbanning after the ban duration expires

#### Configuration

Add rate limiting rules to `configs/rules.json`:

```json
{
  "rule_id": 1010,
  "description": "Rate limit protection - block excessive requests",
  "src_ip": "any",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "any",
  "rate_limit": true,
  "max_requests": 10,
  "time_window_seconds": 1,
  "ban_duration_seconds": 300,
  "action": "drop"
}
```

**Fields:**
- `rate_limit`: Set to `true` to enable rate limiting for this rule
- `max_requests`: Maximum number of requests allowed per time window
- `time_window_seconds`: Time window in seconds (typically 1)
- `ban_duration_seconds`: How long to ban the IP after exceeding the limit
- `action`: What to do when rate limit is exceeded (typically "drop")

#### How It Works

1. Each packet is checked against rate limiting rules first
2. If a rule matches and `rate_limit: true` is set:
   - The system tracks how many requests from this IP in the time window
   - If threshold is exceeded, the IP is banned for the specified duration
   - All packets from a banned IP are dropped
3. Banned IPs are automatically unbanen after the duration expires

#### Example Rule

This rule limits any IP to 10 requests per second, banning for 5 minutes if exceeded:

```json
{
  "rule_id": 1011,
  "description": "Rate limit 10 req/s",
  "src_ip": "any",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "any",
  "rate_limit": true,
  "max_requests": 10,
  "time_window_seconds": 1,
  "ban_duration_seconds": 300,
  "action": "drop"
}
```