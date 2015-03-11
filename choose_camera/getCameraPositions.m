function [camPos camDirection] = getCameraPositions(meshFilename,distance)

if nargin < 2
    distance = 1;
end

[X,T] = readOff(meshFilename);
normals = compute_normal(X,T)';

camPos = X + distance*normals;

camDirection = X-camPos;
directionNorm = sqrt(sum(camDirection.^2,2));
camDirection = camDirection ./ repmat(directionNorm,1,3);