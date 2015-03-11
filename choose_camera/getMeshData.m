function mesh = getMeshData(X,T,numEigs,name)

if nargin < 4
    name = 'mesh';
end

if nargin < 3
    numEigs = 100;
end

mesh = [];
mesh.vertices = X;
mesh.triangles = T;
mesh.name = name;

[mesh.cotLaplacian mesh.areaWeights] = cotLaplacian(X,T);

% Change to negative cot Laplacian and rescale to area = 1
mesh.areaWeights = mesh.areaWeights / sum(mesh.areaWeights);
mesh.cotLaplacian = -1*mesh.cotLaplacian;

mesh.numVertices = size(X,1);
mesh.numTriangles = size(T,1);

evec = [mesh.triangles(:,1) mesh.triangles(:,2)];
evec = [evec; mesh.triangles(:,2) mesh.triangles(:,1)];
evec = [evec; mesh.triangles(:,1) mesh.triangles(:,3)];
evec = [evec; mesh.triangles(:,3) mesh.triangles(:,1)];
evec = [evec; mesh.triangles(:,2) mesh.triangles(:,3)];
evec = [evec; mesh.triangles(:,3) mesh.triangles(:,2)];
evec = unique(evec,'rows');
orderedRows = find(evec(:,1) < evec(:,2));
mesh.edges = evec(orderedRows,:);
mesh.numEdges = size(mesh.edges,1);

% Compute LB eigenstuff
areaMatrix = sparse(1:mesh.numVertices,1:mesh.numVertices,mesh.areaWeights);
[evecs, evals] = eigs(mesh.cotLaplacian, areaMatrix, max(numEigs,100), -1e-5);
evals = diag(evals);

mesh.laplaceBasis = evecs;
mesh.eigenvalues = evals;

% Mean and Gauss curvature, along with per-vertex normals. For now we'll
% compute slowly.
[~,~,~,~,mesh.meanCurvature,mesh.gaussCurvature,mesh.vertexNormals] = compute_curvature(mesh.vertices,mesh.triangles);
mesh.vertexNormals = mesh.vertexNormals';

mesh.conformalFactor = conformalFactor(mesh);