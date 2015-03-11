filename = '167.off';

[X,T] = readOff(filename);
[pos, dir] = getCameraPositions(filename,.1);

patch('vertices',X,'Faces',T,'FaceColor','interp','CData',X(:,2),'edgecolor','none'); 
axis equal; cameratoolbar; hold on; colorbar;

quiver3(pos(:,1),pos(:,2),pos(:,3),dir(:,1),dir(:,2),dir(:,3));

fid = fopen('matrices.txt','w');
for i=1:size(pos,1)
    target = X(i,:);
    camera = pos(i,:);
    v = target - camera;
    v = v/norm(v);

    % normalize the up vector
    up = [0 1 0];
    up = up/norm(up);
    
    % get the orthonormal basis of the projection plane (su):
    s = cross(v,up);
    u = cross(s,v);
    m = [s 0; u 0; -v 0; 0 0 0 1];
    
    % convert from RH to LH system...
    B = eye(4);
    B(3,3) = -1;
    m = B*m*B;
    % the camera position must also be converted after m, so that the translation matches...
    camera(3) = -camera(3);

    % camera transform:
    scale = 1;
    fprintf(fid,'%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n',m);   
    %fprintf(1,'  Translate %f %f %f\n',-camera);
end

dlmwrite('caminfo.txt',[pos dir],' ');