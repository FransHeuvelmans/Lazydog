# SPDX-License-Identifier: EUPL-1.2
import pytest

@pytest.fixture
def db_export_file(tmp_path_factory):
    return tmp_path_factory.mktemp("data") / "out.db"
    