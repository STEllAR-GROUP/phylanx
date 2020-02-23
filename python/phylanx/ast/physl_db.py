# Copyright (c) 2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import os
import datetime
import pickle
import sqlite3


class db:
    """Represents the PhySL cache data for a given script file"""

    @staticmethod
    def file_mod_date(filename):
        t = os.path.getmtime(filename)
        return datetime.datetime.fromtimestamp(t)

    @staticmethod
    def ensure_db(name, version):
        """create new database if it does not exists yet,
           open existing database otherwise"""

        # generate db name from given script name
        head, tail = os.path.split(os.path.realpath(name))
        filename, _ = os.path.splitext(tail)
        dbname = '%s/__physlcache__/%s.db' % (head, filename)

        if not os.path.exists(dbname):
            # make sure directory exists
            dbdir = '%s/__physlcache__' % head
            if not os.path.exists(dbdir):
                os.makedirs(dbdir)

            # database does not exist, create
            db = sqlite3.connect(
                dbname, detect_types=sqlite3.PARSE_DECLTYPES)
            c = db.cursor()

            # create table 'version'
            c.execute("""
                CREATE TABLE version (
                    version INTEGER,
                    created TEXT)
            """)
            c.execute("""
                INSERT INTO version
                    (version, created)
                    VALUES(?, ?)
            """, (version, datetime.datetime.now()))

            # create table 'functions'
            c.execute("""
                CREATE TABLE functions (
                    funcname TEXT PRIMARY KEY,
                    physl    TEXT NOT NULL,
                    ast      TEXT NOT NULL)
            """)

            c.close()
            db.commit()

            return dbname, db, True

        # assume db exists, simply connect
        return dbname, sqlite3.connect(dbname), False

    def created(self):
        """Retrieve the date/time of creation of this database"""

        c = self.db.cursor()

        c.execute("""
            SELECT version, created FROM version
        """)

        rows = c.fetchall()
        c.close()

        if len(rows) == 0:
            raise RuntimeError(
                "database '%s' does not contain any version information" % (
                    self.dbname))

        time = datetime.datetime.strptime(rows[0][1], '%Y-%m-%d %H:%M:%S.%f')
        return rows[0][0], time

    def __init__(self, name, timestamp=None, version=1):
        """create/open database"""

        if timestamp is None:
            timestamp = self.file_mod_date(name)

        self.dbname, self.db, created = self.ensure_db(name, version)

        if not created:
            # the database is older than the given timestamp, recreate it

            db_version, db_created = self.created()
            if db_version < version or \
               db_created < timestamp or \
               db_created < self.file_mod_date(__file__):

                self.db.close()
                os.remove(self.dbname)

                self.dbname, self.db, _ = self.ensure_db(name, version)

    def close(self):
        self.db.close()

    def insert(self, funcname, physl, ast):
        """Insert or update data for a physl function into database"""

        c = self.db.cursor()
        asts = pickle.dumps(ast)
        try:
            # try inserting new record
            c.execute("""
                INSERT INTO functions
                    (funcname, physl, ast)
                    VALUES (?, ?, ?)
            """, (funcname, physl, asts))

        except sqlite3.IntegrityError:
            # record already exists, update values
            c.execute("""
                UPDATE functions SET
                    physl=(?),
                    ast=(?)
                WHERE
                    funcname=('{funcname}')
            """.format(funcname=funcname), (physl, asts))

        c.close()
        self.db.commit()

    def select(self, funcname):
        """find data for the given physl function"""

        c = self.db.cursor()

        c.execute("""
            SELECT physl, ast FROM functions WHERE funcname='{funcname}'
        """.format(funcname=funcname))

        rows = c.fetchall()
        c.close()

        if len(rows) == 0:
            return (None, None)

        return rows[0][0], pickle.loads(rows[0][1])
