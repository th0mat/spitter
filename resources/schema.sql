--
-- PostgreSQL database dump
--

-- Dumped from database version 9.5.2
-- Dumped by pg_dump version 9.5.2

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

--
-- Name: db_size; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW db_size AS
 SELECT pg_database_size('test1'::name) AS pg_database_size;


ALTER TABLE db_size OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: periods; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE periods (
    session_id bigint,
    period_id bigint NOT NULL,
    no_pkts_valid bigint,
    no_pkts_corr bigint,
    bytes_valid bigint,
    bytes_corr bigint,
    no_stations integer,
    end_time timestamp without time zone
);


ALTER TABLE periods OWNER TO postgres;

--
-- Name: COLUMN periods.no_pkts_valid; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN periods.no_pkts_valid IS 'excluding corrupted pkts';


--
-- Name: COLUMN periods.no_pkts_corr; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN periods.no_pkts_corr IS 'corrupted pkts count';


--
-- Name: COLUMN periods.bytes_valid; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN periods.bytes_valid IS 'excluding corrupted pkts';


--
-- Name: COLUMN periods.bytes_corr; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN periods.bytes_corr IS 'corrupted packets';


--
-- Name: summaries; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE summaries (
    summary_id bigint NOT NULL,
    period_id bigint,
    mac_int bigint,
    mac_res character varying(20),
    no_pkts bigint,
    no_bytes bigint
);


ALTER TABLE summaries OWNER TO postgres;

--
-- Name: COLUMN summaries.mac_res; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN summaries.mac_res IS 'mac as resolved at session time';


--
-- Name: COLUMN summaries.no_pkts; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN summaries.no_pkts IS 'number of pkts in period';


--
-- Name: mac_total_traffic; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW mac_total_traffic AS
 SELECT summaries.mac_res AS mac,
    round((sum(summaries.no_pkts) / (1000)::numeric), 0) AS kpkts,
    round((sum(summaries.no_bytes) / (1000000)::numeric), 0) AS mb
   FROM summaries,
    periods
  WHERE ((summaries.period_id = periods.period_id) AND (periods.session_id > 26))
  GROUP BY summaries.mac_res
  ORDER BY (sum(summaries.no_bytes)) DESC;


ALTER TABLE mac_total_traffic OWNER TO postgres;

--
-- Name: macs; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE macs (
    mac_int bigint NOT NULL,
    mac_hex character varying(20),
    mac_res character varying(20)
);


ALTER TABLE macs OWNER TO postgres;

--
-- Name: packets; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE packets (
    pkt_id bigint NOT NULL,
    session_id bigint,
    protocol smallint,
    type smallint,
    subtype smallint,
    to_from_ds smallint,
    crc_valid boolean,
    length integer,
    addr1 bigint,
    addr2 bigint,
    addr3 bigint,
    channel_freq integer,
    comment character varying(30),
    time_stamp timestamp without time zone
);


ALTER TABLE packets OWNER TO postgres;

--
-- Name: COLUMN packets.crc_valid; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN packets.crc_valid IS 'false = corrupted pkt';


--
-- Name: COLUMN packets.length; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN packets.length IS 'packet length in byte (excluding radiotap header)';


--
-- Name: packets_pkt_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE packets_pkt_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE packets_pkt_id_seq OWNER TO postgres;

--
-- Name: packets_pkt_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE packets_pkt_id_seq OWNED BY packets.pkt_id;


--
-- Name: periods_period_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE periods_period_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE periods_period_id_seq OWNER TO postgres;

--
-- Name: periods_period_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE periods_period_id_seq OWNED BY periods.period_id;


--
-- Name: sessions; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE sessions (
    session_id bigint NOT NULL,
    period integer,
    name character varying(15) DEFAULT 'default'::character varying,
    location character varying(15) DEFAULT 'home'::character varying,
    start_time timestamp without time zone
);


ALTER TABLE sessions OWNER TO postgres;

--
-- Name: sessions_session_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE sessions_session_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE sessions_session_id_seq OWNER TO postgres;

--
-- Name: sessions_session_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE sessions_session_id_seq OWNED BY sessions.session_id;


--
-- Name: summaries_summary_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE summaries_summary_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE summaries_summary_id_seq OWNER TO postgres;

--
-- Name: summaries_summary_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE summaries_summary_id_seq OWNED BY summaries.summary_id;


--
-- Name: pkt_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY packets ALTER COLUMN pkt_id SET DEFAULT nextval('packets_pkt_id_seq'::regclass);


--
-- Name: period_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY periods ALTER COLUMN period_id SET DEFAULT nextval('periods_period_id_seq'::regclass);


--
-- Name: session_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sessions ALTER COLUMN session_id SET DEFAULT nextval('sessions_session_id_seq'::regclass);


--
-- Name: summary_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY summaries ALTER COLUMN summary_id SET DEFAULT nextval('summaries_summary_id_seq'::regclass);


--
-- Name: macs_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY macs
    ADD CONSTRAINT macs_pkey PRIMARY KEY (mac_int);


--
-- Name: packets_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY packets
    ADD CONSTRAINT packets_pkey PRIMARY KEY (pkt_id);


--
-- Name: periods_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY periods
    ADD CONSTRAINT periods_pkey PRIMARY KEY (period_id);


--
-- Name: sessions_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sessions
    ADD CONSTRAINT sessions_pkey PRIMARY KEY (session_id);


--
-- Name: summaries_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY summaries
    ADD CONSTRAINT summaries_pkey PRIMARY KEY (summary_id);


--
-- Name: fki_periods; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX fki_periods ON summaries USING btree (period_id);


--
-- Name: fki_session_id; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX fki_session_id ON periods USING btree (session_id);


--
-- Name: fk_periods; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY summaries
    ADD CONSTRAINT fk_periods FOREIGN KEY (period_id) REFERENCES periods(period_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: fk_session; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY periods
    ADD CONSTRAINT fk_session FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

