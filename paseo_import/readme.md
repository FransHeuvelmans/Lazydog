# Paseo extraction instructions

[Paseo][1] is a free and open source step counting app ([src][2]).

Paseo's export is already in an Sqlite format.

To make the data more in line with the rest of Lazydog do (uses an `sqlite3` client)

```{sh}
sqlite3 user.db < transform_datums.sql
```

[1]: https://f-droid.org/en/packages/ca.chancehorizon.paseo/
[2]: https://gitlab.com/pardomi/paseo/tree/HEAD