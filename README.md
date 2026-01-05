# IPS - Intrusion Prevention System

A high-performance, rule-based Intrusion Prevention System (IPS) built in C++ for Linux systems. This IPS leverages `netfilter_queue` to inspect and filter network traffic in real-time, providing advanced security features including deep packet inspection, rate limiting, and flexible rule-based filtering.

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Rule Syntax](#rule-syntax)
- [Examples](#examples)
- [Development & Testing](#development--testing)
- [Troubleshooting](#troubleshooting)

## Features

### Core Capabilities

- **Real-time Packet Inspection**: Intercepts and analyzes network packets using Linux netfilter queue
- **Rule-based Filtering**: Flexible JSON-based rule system with support for complex expressions
- **Deep Packet Inspection**: Content-based filtering with string pattern matching and regex support
- **Protocol Support**: TCP, UDP, and protocol-agnostic filtering
- **IP/Port Filtering**: Source and destination IP address and port filtering with CIDR notation support
- **Rate Limiting**: Intelligent rate limiting with automatic IP banning
- **Logging System**: Comprehensive logging with verbose mode for debugging
- **Multi-threaded**: Efficient packet processing with thread-safe operations

### Advanced Features

#### Rate Limiting & DDoS Protection

The IPS includes sophisticated rate limiting capabilities to protect against DDoS attacks and abuse:

- **Per-IP Request Tracking**: Monitors request rates for individual IP addresses
- **Configurable Thresholds**: Customizable request limits and time windows
- **Automatic Banning**: Temporarily bans IPs that exceed configured thresholds
- **Auto-expiry**: Banned IPs are automatically unbanned after the configured duration
- **Thread-safe Implementation**: Safe operation in multi-threaded environments

#### Expression Parser

Advanced expression parser supporting:

- **Logical Operators**: `AND`, `OR`, `NOT` for complex rule logic
- **Parentheses**: Nested expressions with proper precedence handling
- **Multiple Field Types**: Works with IP addresses, ports, protocols, and string content
- **Case-Insensitive Matching**: String content matching is case-insensitive
- **Flexible Matching**: Supports exact matches, "any" wildcards, and complex expressions

#### Regex Support

- **ECMAScript Regex**: Full regex pattern matching using ECMAScript syntax
- **Case-Insensitive**: All regex patterns are matched case-insensitively
- **Error Handling**: Invalid regex patterns are logged as warnings and treated as non-matching

### Actions

- **DROP**: Silently drop matching packets
- **ALERT**: Log matching packets and allow them through
- **ACCEPT**: Explicitly allow matching packets (default for non-matching)

## Architecture

```
IPS/
├── src/
│   ├── IPS.cpp                      # Main entry point
│   ├── includes.h                   # Common includes
│   ├── packets/
│   │   ├── packet_processor.cpp     # Core packet processing logic
│   │   └── packet_processor.h
│   ├── rules/
│   │   ├── parser.cpp               # Rule parsing and management
│   │   ├── parser.h
│   │   ├── expression_parser.cpp    # Expression evaluation engine
│   │   └── expression_parser.h
│   ├── rate_limit/
│   │   ├── rate_limiter.cpp         # Rate limiting implementation
│   │   └── rate_limiter.h
│   ├── logger/
│   │   ├── logger.cpp               # Logging system
│   │   └── logger.h
│   ├── config/
│   │   ├── config_parser.cpp        # Configuration file parser
│   │   └── config_parser.h
│   └── utils/
│       ├── utils.cpp                # Utility functions
│       └── utils.h
├── configs/
│   ├── config.yml                   # Main configuration file
│   └── rules.json                   # Security rules definition
├── build.sh                         # Build script
└── sample-vuln-app/                 # Test application with vulnerabilities
```

## Requirements

### System Requirements

- **Operating System**: Linux (kernel 2.6.14 or later with netfilter support)
- **Architecture**: x86_64 (amd64) or compatible
- **Privileges**: Root access required for netfilter queue operations

### Dependencies

- **g++**: C++11 compatible compiler
- **libnetfilter-queue**: Netfilter queue library
- **libyaml-cpp**: YAML configuration file parser
- **tclap**: Command-line argument parsing library
- **pthread**: POSIX threads library (typically included)

### Installing Dependencies

#### Arch Linux
```bash
sudo pacman -S libnetfilter_queue yaml-cpp tclap
```

#### Ubuntu/Debian
```bash
sudo apt-get install libnetfilter-queue-dev libyaml-cpp-dev libtclap-dev
```

#### Fedora/RHEL
```bash
sudo dnf install libnetfilter_queue-devel yaml-cpp-devel tclap-devel
```

## Installation

### Building from Source

1. **Clone or download the repository**:
   ```bash
   cd /path/to/IPS
   ```

2. **Build the project**:
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

3. **Verify the build**:
   ```bash
   ls -lh build/IPS
   ```

The compiled binary will be located at `build/IPS`.

## Configuration

### Main Configuration File (`configs/config.yml`)

```yaml
admin:
  username: admin
  password: h3ll0w0rld

web:
  enabled: true

rules:
  path: ./configs/rules.json
  logpath: "./logs/main.log"
```

**Configuration Options:**

| Option | Description | Required | Default |
|--------|-------------|----------|---------|
| `admin.username` | Administrator username (for future web interface) | No | `admin` |
| `admin.password` | Administrator password (change default!) | No | `h3ll0w0rld` |
| `web.enabled` | Enable/disable web interface (future feature) | No | `true` |
| `rules.path` | Path to the rules JSON file | No | `./configs/rules.json` |
| `rules.logpath` | Path to the log file (empty string = no file logging, only stdout/stderr) | No | `""` |

**Note**: The configuration parser supports nested YAML structures. All nested keys are flattened using dot notation (e.g., `rules.path`, `admin.username`).

### Rules Configuration (`configs/rules.json`)

Rules are defined in JSON format with the following structure:

```json
{
  "rule_id": 1001,
  "description": "Rule description",
  "src_ip": "source IP or 'any'",
  "dst_ip": "destination IP or 'any'",
  "src_port": "source port or 'any'",
  "dst_port": "destination port or 'any'",
  "protocol": "TCP|UDP|any",
  "string": "optional string to match in payload",
  "action": "drop|alert|accept"
}
```

**Rate Limiting Rules** include additional fields:

```json
{
  "rule_id": 1010,
  "description": "Rate limit protection",
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

**Note**: When `rate_limit` is enabled, the rule will:
- Track requests per source IP address
- Automatically ban IPs that exceed `max_requests` within `time_window_seconds`
- Ban duration is specified by `ban_duration_seconds` (default: 60 seconds)
- Banned IPs are automatically unbanned after the ban duration expires

## Usage

### Running the IPS

The IPS automatically sets up the required iptables rules on startup and cleans them up on exit. No manual iptables configuration is needed!

**Basic usage:**
```bash
sudo ./build/IPS -c ./configs/config.yml
```

**Verbose mode (recommended for debugging):**
```bash
sudo ./build/IPS -c ./configs/config.yml -v
```

### Command-line Options

- `-c, --config <path>`: Path to configuration file (required)
- `-v, --verbose`: Enable verbose logging mode
- `--help`: Display help message
- `--version`: Display version information

### Stopping the IPS

Press `Ctrl+C` to gracefully stop the IPS. The system will automatically:
- Clean up netfilter queue bindings
- Remove all iptables NFQUEUE rules
- Restore default iptables policies (ACCEPT)
- Free all allocated resources

No manual cleanup is required!

## Rule Syntax

### Rule Structure

Rules are defined as JSON objects in an array. Each rule must be a valid JSON object with the required fields. Rules are processed in the order they appear in the array.

**Field Validation:**
- Unknown fields in rules will generate warnings but won't cause rule parsing to fail
- Duplicate `rule_id` values will cause parsing to fail with an error
- Invalid `action` values will cause parsing to fail with an error
- Missing required fields will cause parsing to fail with an error

### Required Fields

| Field | Description | Examples |
|-------|-------------|----------|
| `rule_id` | Unique identifier (must be unique across all rules) | `1001` |
| `action` | Action to take when rule matches | `"drop"`, `"alert"`, `"accept"` |

**Note**: At least one of `src_ip` or `dst_ip` must be specified (cannot both be missing).

### Basic Fields (Optional with Defaults)

| Field | Description | Default | Examples |
|-------|-------------|---------|----------|
| `description` | Human-readable description | `"unknown"` | `"Block SQL injection"` |
| `src_ip` | Source IP address (supports expressions) | `"any"` | `"192.168.1.100"`, `"any"`, `"192.168.1.0/24 OR 10.0.0.1"` |
| `dst_ip` | Destination IP address (supports expressions) | `"any"` | `"8.8.8.8"`, `"any"`, `"192.168.0.0/16"` |
| `src_port` | Source port (supports expressions) | `"any"` | `"80"`, `"any"`, `"1024-65535"`, `"80 OR 443"` |
| `dst_port` | Destination port (supports expressions) | `"any"` | `"443"`, `"any"`, `"80 OR 443"`, `"(80 OR 443) AND NOT 8080"` |
| `protocol` | Protocol type (supports expressions) | `"any"` | `"TCP"`, `"UDP"`, `"any"`, `"TCP OR UDP"` |

### Content Matching Fields (Optional)

| Field | Description | Example |
|-------|-------------|---------|
| `string` | Pattern to match in payload (supports expressions with OR/AND/NOT, case-insensitive substring matching) | `"select OR union OR drop"` |
| `regex` | Regular expression pattern to match in payload (ECMAScript regex, case-insensitive). Invalid regex patterns are logged as warnings and treated as non-matching. | `".*(union\|select\|drop).*"` |

**Note**: Both `string` and `regex` fields are optional. If specified, the packet payload must match the pattern for the rule to apply. You can use both fields together - both must match for the rule to apply.

### Rate Limiting Fields (Optional)

| Field | Description | Default | Example |
|-------|-------------|---------|---------|
| `rate_limit` | Enable rate limiting for this rule | `false` | `true` |
| `max_requests` | Maximum requests allowed per time window | `10` | `10`, `100` |
| `time_window_seconds` | Time window duration in seconds | `1` | `1`, `60` |
| `ban_duration_seconds` | Duration to ban IP after exceeding rate limit (in seconds) | `60` | `300`, `3600` |

**Rate Limiting Behavior:**
- Rate limiting is applied per source IP address
- When an IP exceeds `max_requests` within `time_window_seconds`, it is automatically banned
- Banned IPs are blocked for `ban_duration_seconds` (all packets from banned IPs are dropped)
- Bans automatically expire after the specified duration
- Rate limiting rules are evaluated before other security rules
- Multiple rate limiting rules can be active simultaneously

### Expression Syntax

The IPS supports complex expressions using logical operators for IP addresses, ports, protocols, and string content:

**Operators:**
- `OR`: Logical OR (either condition matches)
- `AND`: Logical AND (both conditions must match) - implicit between literals
- `NOT`: Logical negation
- `()`: Parentheses for grouping and precedence control

**Operator Precedence:**
1. Parentheses `()`
2. `NOT`
3. `AND` (implicit or explicit)
4. `OR`

**Supported Field Types:**

1. **IP Addresses** (`src_ip`, `dst_ip`):
   - Exact match: `"192.168.1.100"`
   - Any: `"any"` or `"ANY"`
   - Expressions: `"192.168.1.0/24 OR 10.0.0.1"`
   - Note: CIDR notation in expressions is parsed as literal strings; exact IP matching is performed

2. **Ports** (`src_port`, `dst_port`):
   - Exact match: `"80"`, `"443"`
   - Any: `"any"` or `"ANY"`
   - Expressions: `"80 OR 443"`, `"(80 OR 443) AND NOT 8080"`

3. **Protocols** (`protocol`):
   - Exact match: `"TCP"`, `"UDP"`
   - Any: `"any"` or `"ANY"`
   - Expressions: `"TCP OR UDP"`

4. **String Content** (`string`):
   - Case-insensitive substring matching
   - Supports expressions: `"select OR union OR drop"`
   - Multiple patterns: `"select AND union"` (both must be found in payload)
   - Negation: `"NOT admin"` (payload must not contain "admin")

**Examples:**

```json
"src_ip": "192.168.1.0/24 OR 10.0.0.0/8"
"dst_port": "(80 OR 443) AND NOT 8080"
"protocol": "TCP OR UDP"
"string": "select OR union OR drop"
"string": "(admin OR root) AND NOT test"
```

## Examples

### Example 1: Block Specific IP Address

```json
{
  "rule_id": 1001,
  "description": "Block malicious IP",
  "src_ip": "192.0.2.100",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "any",
  "action": "drop"
}
```

### Example 2: SQL Injection Protection (String Pattern)

```json
{
  "rule_id": 1003,
  "description": "Block SQL injection attempts",
  "src_ip": "any",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "TCP",
  "string": "select OR union OR drop",
  "action": "drop"
}
```

### Example 2b: SQL Injection Protection (Regex Pattern)

```json
{
  "rule_id": 1002,
  "description": "Block SQL injection patterns by regex",
  "src_ip": "any",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "TCP",
  "regex": ".*(union|select|drop|delete|insert).*",
  "action": "drop"
}
```

### Example 3: Rate Limiting for DDoS Protection

```json
{
  "rule_id": 1010,
  "description": "Rate limit - 10 requests per second",
  "src_ip": "any",
  "dst_ip": "127.0.0.1",
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

### Example 4: Complex Rule with Expressions

```json
{
  "rule_id": 1005,
  "description": "Monitor internal network web traffic",
  "src_ip": "192.168.1.0/24 OR 10.0.0.1",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "(80 OR 443) AND NOT 8080",
  "protocol": "TCP OR UDP",
  "action": "alert"
}
```

### Example 4b: Combined String and Regex Matching

```json
{
  "rule_id": 1006,
  "description": "Block suspicious SQL patterns",
  "src_ip": "any",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "TCP",
  "string": "admin OR root",
  "regex": ".*(union|select).*",
  "action": "drop"
}
```

**Note**: When both `string` and `regex` are specified, both patterns must match for the rule to apply.

### Example 5: Alert on Suspicious Activity

```json
{
  "rule_id": 1004,
  "description": "Log potential SQL injection",
  "src_ip": "any",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "TCP",
  "string": "select OR union OR drop",
  "action": "alert"
}
```

**Note**: `alert` action logs the matching packet but allows it through. Use this for monitoring before switching to `drop` action.

### Example 6: Accept Rule (Whitelist)

```json
{
  "rule_id": 1007,
  "description": "Allow trusted IP range",
  "src_ip": "192.168.1.0/24",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "any",
  "protocol": "any",
  "action": "accept"
}
```

**Note**: Rules are evaluated in order. The first matching rule's action is applied (except for `alert` which continues evaluation).

## Development & Testing

### Testing with the Vulnerable Sample Application

The project includes a vulnerable web application for testing:

```bash
# Navigate to the sample app directory
cd sample-vuln-app

# Install Flask if not already installed
pip install flask

# Run the vulnerable application
python main.py
```

The application will start on `http://localhost:5000` with intentional vulnerabilities:
- SQL injection
- Command injection
- XSS (Cross-site scripting)
- Path traversal

**Testing Workflow:**

1. Start the vulnerable app
2. Configure IPS rules to protect against specific attacks
3. Run the IPS in verbose mode (it will automatically set up iptables)
4. Attempt attacks and observe IPS behavior
5. Stop the IPS with Ctrl+C (it will automatically clean up iptables)

### Rate Limiting Test

```bash
# Install a HTTP benchmarking tool
sudo pacman -S apache-tools  # or apt-get install apache2-utils

# Test rate limiting (send 100 requests)
ab -n 100 -c 10 http://localhost:5000/

# Observe IPS logs to see rate limiting in action
```

## Troubleshooting

### Common Issues

**Issue: Permission denied**
```
Solution: Run the IPS with sudo privileges:
sudo ./build/IPS -c ./configs/config.yml
```

**Issue: "Cannot open library: libnetfilter_queue.so"**
```
Solution: Install the libnetfilter-queue library:
sudo pacman -S libnetfilter_queue
```

**Issue: "Failed to load configuration"**
```
Solution: Verify the config file path and format:
- Check that config.yml exists
- Validate YAML syntax
- Ensure proper file permissions
- Verify nested keys use proper indentation
```

**Issue: "Duplicate rule ID"**
```
Solution: Ensure each rule has a unique rule_id:
- Check rules.json for duplicate rule_id values
- Rule IDs must be unique across all rules
```

**Issue: "Invalid regex pattern"**
```
Solution: Verify regex syntax:
- Check regex patterns in rules.json
- Ensure proper escaping (e.g., use \| for OR in regex, not |)
- Test regex patterns separately before adding to rules
- Invalid regex patterns are logged as warnings and won't match
```

**Issue: No packets being processed**
```
Solution: The IPS sets up iptables automatically. Verify rules are active:
sudo iptables -L -n -v | grep NFQUEUE

If no rules are shown, the IPS may not have started properly. Check logs.
```

**Issue: "Cannot bind to netfilter queue"**
```
Solution: Ensure no other process is using the queue:
- Check for other running IPS instances
- Verify queue number (default: 0)
```

### Debug Mode

Run with verbose flag to see detailed packet processing:

```bash
sudo ./build/IPS -c ./configs/config.yml -v
```

This will display:
- Loaded rules
- Packet details (IP, port, protocol)
- Rule matching decisions
- Rate limiting events
- Banned IP addresses

### Logs Location

Logs are written to:
- **Standard output (stdout)**: INFO and ALERT messages
- **Standard error (stderr)**: ERROR and WARNING messages
- **Log file** (if `rules.logpath` is configured in `config.yml`): All log messages

**Log Levels:**
- `INFO`: General information (startup, rule loading, etc.)
- `ALERT`: Security alerts (rule matches, rate limit violations, banned IPs)
- `WARNING`: Non-critical issues (unknown rule fields, invalid regex patterns)
- `ERROR`: Critical errors (configuration errors, rule validation failures)

**Verbose Mode:**
When `-v` flag is used, additional details are logged:
- All loaded rules with full details
- Packet processing details (IP, port, protocol)
- Rule matching decisions
- Rate limiting events
- Banned IP addresses

Redirect logs to a file:
```bash
sudo ./build/IPS -c ./configs/config.yml -v 2>&1 | tee ips.log
```

Or configure file logging in `config.yml`:
```yaml
rules:
  logpath: "./logs/main.log"
```

**Note**: The logger automatically creates the log directory if it doesn't exist. Log files are opened in append mode, so logs are preserved across IPS restarts.

## Architecture Details

### Startup and Initialization

On startup, the IPS automatically:
1. Checks for root privileges
2. Registers signal handlers (SIGINT, SIGTERM, SIGQUIT) for graceful shutdown
3. **Sets up iptables rules** to redirect traffic to netfilter queue 0:
   - `INPUT` chain → NFQUEUE
   - `OUTPUT` chain → NFQUEUE
   - `FORWARD` chain → NFQUEUE (for router/gateway mode)
4. Opens and binds to netfilter queue
5. Loads configuration and rules
6. Initializes rate limiter

On shutdown (Ctrl+C), the IPS automatically:
1. Flushes all NFQUEUE rules from INPUT, OUTPUT, and FORWARD chains
2. Restores default iptables policies to ACCEPT
3. Closes netfilter queue handle
4. Frees all resources

### Packet Processing Flow

```
1. Packet arrives at netfilter queue
2. IPS receives packet data
3. Extract IP, port, protocol information from packet headers
4. Check rate limiting rules first (if any rate_limit rules exist)
   - If IP is currently banned → DROP immediately
   - Evaluate rate limit rules in order
   - If rate limit exceeded → BAN IP and apply rule action (usually DROP)
5. Evaluate security rules in order (if not already handled by rate limiting)
   - Match IP addresses (src_ip, dst_ip) using expression parser
   - Match ports (src_port, dst_port) using expression parser
   - Match protocol using expression parser
   - Check string patterns in payload (if `string` field specified)
   - Check regex patterns in payload (if `regex` field specified)
   - If rule matches:
     - If action is "alert" → Log and continue to next rule
     - Otherwise → Apply action (drop/accept) and stop evaluation
6. If no rule matches → ACCEPT (default behavior)
7. Return verdict to netfilter (ACCEPT or DROP)
```

**Important Notes:**
- Rules are evaluated in the order they appear in `rules.json`
- `alert` actions log the match but continue rule evaluation
- `drop` and `accept` actions stop rule evaluation immediately
- Rate limiting rules are always checked first, before other security rules
- String and regex matching are case-insensitive
- Both `string` and `regex` fields must match if both are specified

### Thread Safety

- Rate limiter uses mutexes for thread-safe operations
- Rule parsing is performed at startup (read-only during runtime)
- Packet processing can be multi-threaded (future enhancement)

## Performance Considerations

- **Optimized String Matching**: Uses efficient string search algorithms for substring matching
- **Rule Ordering**: Place most common rules first for faster matching (first match stops evaluation)
- **Rate Limiter**: Uses hash maps for O(1) lookup performance with automatic cleanup of old entries
- **Memory Efficient**: Minimal memory footprint per packet, automatic cleanup of expired rate limit entries
- **Regex Performance**: Regex patterns are compiled once per rule, but complex patterns may impact performance
- **Expression Parsing**: Expressions are parsed once per rule evaluation, consider rule ordering for performance

## Security Best Practices

1. **Change Default Passwords**: Update the admin password in `config.yml`
2. **Regular Rule Updates**: Review and update rules based on threat landscape
3. **Log Monitoring**: Regularly review logs for suspicious activity and rate limit violations
4. **Test Before Production**: Test new rules in `alert` mode before using `drop` action
5. **Backup Configuration**: Keep backups of working configurations
6. **Least Privilege**: Run with minimal required privileges (root required for netfilter)
7. **Rule Validation**: Ensure all rules have unique `rule_id` values and valid syntax
8. **Rate Limiting**: Configure appropriate rate limits to prevent DDoS while avoiding false positives
9. **Regex Safety**: Test regex patterns thoroughly; invalid patterns won't match but may indicate configuration errors
10. **File Permissions**: Secure configuration files with appropriate permissions (read-only for non-root users)

## Future Enhancements

- [ ] Web-based management interface
- [ ] Real-time statistics dashboard
- [ ] Database integration for log storage
- [ ] Machine learning-based anomaly detection
- [ ] IPv6 support
- [ ] Custom plugin system
- [ ] Distributed deployment support
- [ ] Performance metrics and monitoring

## License

This project is provided as-is for educational and security purposes.

## Contributing

Contributions are welcome! Please ensure:
- Code follows existing style conventions
- New features include appropriate documentation
- Security features are thoroughly tested

## Author

Developed as part of cybersecurity research and education.

---

**Note**: This IPS is designed for educational purposes and testing environments. For production deployments, conduct thorough testing and security audits.
