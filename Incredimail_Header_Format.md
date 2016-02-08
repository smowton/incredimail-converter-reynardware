# Introduction #

My current knownledge of the Incredimail header format

## Details ##

| **Name of Field** | **Size in Bytes** | **Notes** |
|:------------------|:------------------|:----------|
| Header (1)        | 4                 | This can be the header version or size of the previous record |
| Header (2)        | 8                 | Always:3C 12 08 49 17 09 45 3E |
| Deleted Field     | 1                 | 01 - normal and 02 - marked deleted |
| Unknown           | 3                 | 00 00 00  |
| Unread            | 1                 | 00 for off 01 for on |
| Flagged           | 1                 | 00 for off 01 for on |
| Unknown           | 14                |
| Length of “To:”   | 4                 |
| “To” strings      | See previous field | v5 is Unicode |
| Length of “From:” | 4                 |
| “From” string     | See previous field | v5 is Unicode |
| Length of “Subject:” | 4                 |
| “Subject:” string | See previous field | v5 is Unicode |
| Sent Time         | 4                 | Unix Time (UTC) |
| Received Time     | 4                 | Unix Time (UTC) |
| Unknown           | 4                 | Attachments? |
| Pop server size   | 4                 |
| Pop server string | See previous field | v5 is Unicode |
| Unknown           | 6                 | Attachments? |
| Size of file      | 4                 |
| Offset in the database | 4                 |
| Unknown           | 4                 |