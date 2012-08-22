\set VERBOSITY terse
set client_min_messages to ERROR;

-- Import city_data
\i load_topology.sql

SELECT topology.ST_ModEdgeHeal('city_data', 1, null);
SELECT topology.ST_ModEdgeHeal('city_data', null, 1);
SELECT topology.ST_ModEdgeHeal(null, 1, 2);
SELECT topology.ST_ModEdgeHeal('', 1, 2);

-- Not connected edges
SELECT topology.ST_ModEdgeHeal('city_data', 25, 3);

-- Other connected edges
SELECT topology.ST_ModEdgeHeal('city_data', 9, 10);

-- Closed edge
SELECT topology.ST_ModEdgeHeal('city_data', 2, 3);
SELECT topology.ST_ModEdgeHeal('city_data', 3, 2);

-- Heal to self 
SELECT topology.ST_ModEdgeHeal('city_data', 25, 25);

-- Good ones {

-- check state before 
SELECT 'E'||edge_id,
  ST_AsText(ST_StartPoint(geom)), ST_AsText(ST_EndPoint(geom)),
  next_left_edge, next_right_edge, start_node, end_node
  FROM city_data.edge_data ORDER BY edge_id;
SELECT 'N'||node_id FROM city_data.node;

-- No other edges involved, SQL/MM caseno 2, drops node 6
SELECT 'MH(4,5)', topology.ST_ModEdgeHeal('city_data', 4, 5);

-- Face and other edges involved, SQL/MM caseno 1, drops node 16
SELECT 'MH(21,6)', topology.ST_ModEdgeHeal('city_data', 21, 6);
-- Face and other edges involved, SQL/MM caseno 2, drops node 19
SELECT 'MH(8,15)', topology.ST_ModEdgeHeal('city_data', 8, 15);
-- Face and other edges involved, SQL/MM caseno 3, drops node 8
SELECT 'MH(12,22)', topology.ST_ModEdgeHeal('city_data', 12, 22);
-- Face and other edges involved, SQL/MM caseno 4, drops node 11
SELECT 'MH(16,14)', topology.ST_ModEdgeHeal('city_data', 16, 14);

-- check state after
SELECT 'E'||edge_id,
  ST_AsText(ST_StartPoint(geom)), ST_AsText(ST_EndPoint(geom)),
  next_left_edge, next_right_edge, start_node, end_node
  FROM city_data.edge_data ORDER BY edge_id;
SELECT 'N'||node_id FROM city_data.node;

-- }

-- clean up
SELECT topology.DropTopology('city_data');

-------------------------------------------------------------------------
-------------------------------------------------------------------------
-------------------------------------------------------------------------

-- Now test in presence of features 

SELECT topology.CreateTopology('t') > 1;
CREATE TABLE t.f(id varchar);
SELECT topology.AddTopoGeometryColumn('t', 't', 'f','g', 'LINE');

SELECT 'E'||topology.AddEdge('t', 'LINESTRING(2 2, 2  8)');        -- 1
SELECT 'E'||topology.AddEdge('t', 'LINESTRING(2  8,  8  8)');      -- 2

INSERT INTO t.f VALUES ('F+E1',
  topology.CreateTopoGeom('t', 2, 1, '{{1,2}}')); 

-- This should be forbidden, as F+E1 above could not be
-- defined w/out one of the edges
SELECT topology.ST_ModEdgeHeal('t', 1, 2);
SELECT topology.ST_ModEdgeHeal('t', 2, 1);

-- This is for ticket #941
SELECT topology.ST_ModEdgeHeal('t', 1, 200);
SELECT topology.ST_ModEdgeHeal('t', 100, 2);

-- Now see how signed edges are updated

SELECT 'E'||topology.AddEdge('t', 'LINESTRING(0 0, 5 0)');         -- 3
SELECT 'E'||topology.AddEdge('t', 'LINESTRING(10 0, 5 0)');        -- 4

INSERT INTO t.f VALUES ('F+E3-E4',
  topology.CreateTopoGeom('t', 2, 1, '{{3,2},{-4,2}}')); 
INSERT INTO t.f VALUES ('F-E3+E4',
  topology.CreateTopoGeom('t', 2, 1, '{{-3,2},{4,2}}')); 

SELECT r.topogeo_id, r.element_id
  FROM t.relation r, t.f f WHERE 
  r.layer_id = layer_id(f.g) AND r.topogeo_id = id(f.g)
  AND r.topogeo_id in (2,3)
  ORDER BY r.layer_id, r.topogeo_id;

-- This is fine, but will have to tweak definition of
-- 'F+E3-E4' and 'F-E3+E4'
SELECT 'MH(3,4)', topology.ST_ModEdgeHeal('t', 3, 4);

-- This is for ticket #942
SELECT topology.ST_ModEdgeHeal('t', 1, 3);

SELECT r.topogeo_id, r.element_id
  FROM t.relation r, t.f f WHERE 
  r.layer_id = layer_id(f.g) AND r.topogeo_id = id(f.g)
  AND r.topogeo_id in (2,3)
  ORDER BY r.layer_id, r.topogeo_id;

SELECT topology.DropTopology('t');

-------------------------------------------------------------------------
-------------------------------------------------------------------------
-------------------------------------------------------------------------

-- TODO: test registered but unexistent topology
-- TODO: test registered but corrupted topology
--       (missing node, edge, relation...)
