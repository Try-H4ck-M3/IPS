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
- **Deep Packet Inspection**: Content-based filtering with string pattern matching
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
- **CIDR Notation**: Subnet matching (e.g., `192.168.1.0/24`)
- **Port Ranges**: Flexible port matching with ranges and wildcards

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
  password: your_secure_password

web:
  enabled: true
```

**Configuration Options:**
- `admin.username`: Administrator username (for future web interface)
- `admin.password`: Administrator password (change default!)
- `web.enabled`: Enable/disable web interface (future feature)

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

### Basic Fields

| Field | Description | Examples |
|-------|-------------|----------|
| `rule_id` | Unique identifier | `1001` |
| `description` | Human-readable description | `"Block SQL injection"` |
| `src_ip` | Source IP address | `"192.168.1.100"`, `"any"`, `"10.0.0.0/24"` |
| `dst_ip` | Destination IP address | `"8.8.8.8"`, `"any"`, `"192.168.0.0/16"` |
| `src_port` | Source port | `"80"`, `"any"`, `"1024-65535"` |
| `dst_port` | Destination port | `"443"`, `"any"`, `"80 OR 443"` |
| `protocol` | Protocol type | `"TCP"`, `"UDP"`, `"any"` |
| `action` | Action to take | `"drop"`, `"alert"`, `"accept"` |

### Optional Fields

| Field | Description | Example |
|-------|-------------|---------|
| `string` | Pattern to match in payload | `"select OR union OR drop"` |

### Rate Limiting Fields

| Field | Description | Example |
|-------|-------------|---------|
| `rate_limit` | Enable rate limiting | `true` |
| `max_requests` | Max requests per time window | `10` |
| `time_window_seconds` | Time window in seconds | `1` |
| `ban_duration_seconds` | Ban duration in seconds | `300` |

### Expression Syntax

The IPS supports complex expressions using logical operators:

**Operators:**
- `OR`: Logical OR (either condition matches)
- `AND`: Logical AND (both conditions must match)
- `NOT`: Logical negation
- `()`: Parentheses for grouping

**Examples:**

```json
"src_ip": "192.168.1.0/24 OR 10.0.0.0/8"
"dst_port": "(80 OR 443) AND (NOT 8080)"
"protocol": "TCP OR UDP"
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

### Example 2: SQL Injection Protection

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
  "src_ip": "192.168.1.0/24 OR (10.0.0.1 AND 10.0.0.2)",
  "dst_ip": "any",
  "src_port": "any",
  "dst_port": "(80 OR 443) AND (NOT 8080)",
  "protocol": "TCP OR UDP",
  "action": "alert"
}
```

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
- Standard output (stdout) for INFO messages
- Standard error (stderr) for ERROR messages

Redirect logs to a file:
```bash
sudo ./build/IPS -c ./configs/config.yml -v 2>&1 | tee ips.log
```

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
3. Extract IP, port, protocol information
4. Check rate limiting rules first
   - If IP is banned → DROP
   - If rate limit exceeded → BAN IP and DROP
5. Evaluate security rules in order
   - Match IP/port/protocol
   - Check string patterns (if specified)
   - Apply first matching rule action
6. If no rule matches → ACCEPT (default)
7. Return verdict to netfilter
```

### Thread Safety

- Rate limiter uses mutexes for thread-safe operations
- Rule parsing is performed at startup (read-only during runtime)
- Packet processing can be multi-threaded (future enhancement)

## Performance Considerations

- **Optimized String Matching**: Uses efficient string search algorithms
- **Rule Ordering**: Place most common rules first for faster matching
- **Rate Limiter**: Uses hash maps for O(1) lookup performance
- **Memory Efficient**: Minimal memory footprint per packet

## Security Best Practices

1. **Change Default Passwords**: Update the admin password in `config.yml`
2. **Regular Rule Updates**: Review and update rules based on threat landscape
3. **Log Monitoring**: Regularly review logs for suspicious activity
4. **Test Before Production**: Test new rules in alert mode before using drop
5. **Backup Configuration**: Keep backups of working configurations
6. **Least Privilege**: Run with minimal required privileges

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
